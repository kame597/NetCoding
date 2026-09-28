#include "http_response.h"

HttpResponse::HttpResponse():
    status_code_(StatusCode::kUnknown), version_("HTTP/1.1")
{}

HttpResponse::~HttpResponse(){
}

void HttpResponse::set_status_code(StatusCode code){
    status_code_ = code;
}
HttpResponse::StatusCode HttpResponse::status_code() const{
    return status_code_;
};

std::string HttpResponse::status_message() const{
    switch(status_code_){
        case StatusCode::k200Ok:
            return "OK";
        case StatusCode::k204NoContent:
            return "No Content";
        case StatusCode::k301MovedPermanently:
            return "Moved Permanently";
        case StatusCode::k400BadRequest:
            return "Bad Request";
        case StatusCode::k403Forbidden:
            return "Forbidden";
        case StatusCode::k404NotFound:
            return "Not Found";
        case StatusCode::k405MethodNotAllowed:
            return "Method Not Allowed";
        case StatusCode::k500InternalServerError:
            return "Internal Server Error";
        default:
            return "Unknown";
    }
}

void HttpResponse::set_version(const std::string & version){
    version_ = version;
}

const std::string& HttpResponse::version() const{
    return version_;
}

void HttpResponse::add_header(const std::string &field, const std::string &value){
    header_[field] = value;
}

std::string HttpResponse::get_header_value(const std::string &field) const{
    auto it = header_.find(field);
   if(it  == header_.end())
        return "";
   else{
        return it->second;
   }
}

const std::map<std::string, std::string>& HttpResponse::header() const{
    return header_;
}

void HttpResponse::set_body(const std::string &body){
    body_ = body;
}

const std::string& HttpResponse::body() const{
    return body_;
}

std::string HttpResponse::ToString()const{
    std::string str;
    auto space = " ";
    auto crlf = "\r\n";
    // 状态行拼接
    str = version() + space + std::to_string(static_cast<int>(status_code_)) + space + status_message() + crlf;

    /*
    *   首部行拼接
    *   注：Content-Length需要由response框架负责，防止业务层插入了错误的内容长度
    *   内容长度是必须的，因为客户端需要内容长度来计算报文何时结束
    *   同理，Connection必须由response接管，用于决定连接的生命周期语义，如果由业务层负责可能导致行为不一致
    */ 
    for(const auto& pair : header_){
        if(pair.first == "Content-Length" || pair.first == "content-length"
            || pair.first == "Connection" || pair.first == "connection")
                continue;
        str += pair.first + ":" + space + pair.second + crlf;
    }

    if(status_code_ != StatusCode::k204NoContent)   // 204的语义就是不存在content-length，因此需要过滤204状态码
        str += "Content-Length: " + std::to_string(body_.size()) + crlf;
    str += "Connection: close\r\n";     // 假设使用的是短连接

    str += crlf;    //空行
    str += body_;
    return str;
}

void HttpResponse::Clear(){
    status_code_ = StatusCode::kUnknown;
    version_ = "HTTP/1.1";
    header_.clear();
    body_ = "";
}

/*
*   HTTP/1.1 200 OK\r\n			   状态行
*   Content-Encoding: gzip\r\n	   响应头	
*   Content-Type: text/html\r\n
*   Content-Length: 5\r\n
*   \r\n						       空行
*   hello						       响应正文
*   status_code_ = 200
*   version_ = HTTP/1.1
*   header_ = {"Content-Encoding" -> "gzip", "Content-Type" -> "text", ...}
*   body_ = "hello"
*/