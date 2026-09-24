#include "http_context.h"
#include <algorithm>

HttpContext::HttpContext():state_(HttpRequestParseState::kExpectRequestLine){

}

HttpContext::~HttpContext(){
}

/*
*   @brief
*   ParseRequest为状态机主循环，持续处理报文内容
*/
HttpContext::ParseResult HttpContext::ParseRequest(Buffer *buf){
    while(state_ != HttpRequestParseState::kGotAll){
        ParseState res = ParseState::kError;
        switch(state_){
            case HttpRequestParseState::kExpectRequestLine:
                res = ParseRequestLine(buf);
                if(res == ParseState::kOk) state_ = HttpRequestParseState::kExpectHeaderLine;
                break;
            case HttpRequestParseState::kExpectHeaderLine:
                res = ParseHeaders(buf);
                if(res == ParseState::kOk) state_ = HttpRequestParseState::kExpectBody;
                break;
            case HttpRequestParseState::kExpectBody:
                res = ParseBody(buf);
                if(res == ParseState::kOk) state_ = HttpRequestParseState::kGotAll;
                break;
            case HttpRequestParseState::kGotAll:
                break;
        }

        // 在循环过程中检测，如果res被置为非kOk，则直接从循环中断返回
        if(res == ParseState::kError) return ParseResult::kError;
        if(res == ParseState::kNeedMore) return ParseResult::kNeedMore;
    }
    
    // 如果安全结束循环，则返回正常值
    return ParseResult::kGotAll;
}

void HttpContext::Reset(){
    state_ = HttpRequestParseState::kExpectRequestLine;
    request_.Clear();
}

HttpContext::ParseState HttpContext::ParseRequestLine(Buffer * buf){
    auto crlf = buf->FindCRLF();
    if(!crlf) return ParseState::kNeedMore; //如果当前请求行被截断，则等待下次数据

    auto start = buf->Peek(); 
    auto space1 = std::find(start, crlf, ' ');  //注：' '代表char，而" "代表char[2]
    if(space1 == crlf) return ParseState::kError;
    
    auto space2 = std::find(space1 + 1, crlf, ' ');
    if(space2 == crlf) return ParseState::kError;

    std::string method = std::string(start, space1);
    request_.set_method(method);
    if(request_.method() == HttpRequest::Method::kInvalid) return ParseState::kError;    //对畸形报文的校验

    std::string raw_url = std::string(space1 + 1, space2);
    if(!ParseUrl(raw_url)) return ParseState::kError;

    std::string version = std::string(space2 + 1, crlf);
    request_.set_version(version);
    if(request_.version() == HttpRequest::Version::kUnknown) return ParseState::kError;

    buf->RetrieveUntil(crlf + 2);   //消费掉指定数据
    return ParseState::kOk;
}

HttpContext::ParseState HttpContext::ParseHeaders(Buffer * buf){
    // 判定结束的标志，即当前读取数据的起点就是CRLF，即crlf == start
    while(true){
        auto crlf = buf->FindCRLF();
        if(!crlf) return ParseState::kNeedMore;

        auto start = buf->Peek();
        if(crlf == start){  //crlf == start，则解析首部结束
            buf->RetrieveUntil(crlf + 2);
            return ParseState::kOk;
        }

        auto colon = std::find(start, crlf, ':');   //找到冒号位置
        if(colon == crlf) return ParseState::kError;
        std::string field(start, colon);

        // 关于首部行的空格的处理是必要的，因为空格的数量是不固定的，不能用硬编码的方式处理
        // find_if_not: 在给定范围内，返回第一个不符合谓词的字符指针
        auto value_begin = std::find_if_not(colon + 1, crlf, [](char c){
            return c == ' ' || c == '\t';
        });
        // 首部行的值是允许为空的，因此无需错误处理
        // if(value_begin == crlf) return ParseState::kError;

        std::string value(value_begin, crlf);
        // if(value.empty()) return ParseState::kError;

        request_.add_header(field, value);

        buf->RetrieveUntil(crlf + 2);
    }
}

/*
*   ParseBody
*   @brief
*   解析报文主体，因为没有crlf界定，因此需要用首部行的Content-Length来界定
*/
HttpContext::ParseState HttpContext::ParseBody(Buffer *buf){
    std::string len_str = request_.get_header_value("content-length");
    if(len_str.empty())
        return ParseState::kOk; //长度字段为空，说明body为空，正常返回
    
    size_t content_length = 0;
    try{
        size_t pos = 0;
        unsigned long val = std::stoul(len_str, &pos);  //pos用于存储stoul的返回位置
        //如果pos 不等于原字符串长度，说明原字符串存在非数字字符，说明报文错误
        if(pos != len_str.size()) return ParseState::kError;
        content_length = val;
    }catch(const std::exception&){  //捕获stoul的两种异常：一种是开头非数字，另一种是数值超出范围
        return ParseState::kError;
    }

    if(buf->ReadableBytes() < content_length)
        return ParseState::kNeedMore;
    
    request_.set_body(std::string(buf->Peek(), buf->Peek() + content_length));
    buf->Retrieve(content_length);
    return ParseState::kOk;
}

bool HttpContext::ParseUrl(const std::string &raw_url){
    auto qmark = raw_url.find('?');
    auto path = raw_url.substr(0, qmark);
    if(path.empty() || path[0] != '/' ) return false;
    request_.set_url(path);

    if(qmark == std::string::npos) return true; //如果没有?，说明只有url，没有查询参数，可以直接返回

    auto query = raw_url.substr(qmark + 1); //获得查询参数串，比如q=cpp&page=2
    size_t start = 0;
    while(start <= query.size()){
        auto amp = query.find('&', start);  //查找查询参数串里的&符号位置
        if(amp == std::string::npos) amp = query.size();

        std::string pair = query.substr(start, amp - start); //从查询参数串找一个子串配对，比如q=cpp
        if(!pair.empty()){
            auto eq = pair.find('=');
            if(eq == std::string::npos){
                request_.set_request_params(pair, "");
            }
            else{
                request_.set_request_params(pair.substr(0, eq), pair.substr(eq + 1));
            }
        }

        start = amp + 1; //跳过该&，现在start指向的是下一个配对的首字符，比如page=2的p
    }
    return true;
}

/*
*   示例报文与成员对应关系：
*   POST /search?q=cpp&page=2 HTTP/1.1\r\n     ← 请求行（Request Line）
*   Host: example.com\r\n                      ← 头部字段（Header）
*   Content-Type: application/x-www-form-urlencoded\r\n
*   Content-Length: 11\r\n
*   \r\n                                       ← 空行，分隔头部和正文
*   q=hello&x=1                                ← 正文（Body）
*   
*   其中：
*   method_ = POST
*   version_ = HTTP/1.1
*   url = /search
*   request_params = {"q"->"cpp", "page"->"2"}
*   header = {"Host"->"example.com", "Content_type"->"...", ...}
*   body = "q=hello&x=1"
*/
