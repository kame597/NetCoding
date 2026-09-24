#pragma once
#include <string>
#include <memory>
#include "http_request.h"
#include "common.h"
#include "buffer.h"


class HttpContext{
public:
    DISALLOW_COPY_AND_MOVE(HttpContext);
    enum class HttpRequestParseState{   //HTTP请求解析状态
        kExpectRequestLine,     //请求行    
        kExpectHeaderLine,  //首部行
        kExpectBody,    //请求实体
        kGotAll     //收集完毕
    };

    enum class ParseResult{
        kNeedMore,
        kGotAll,
        kError
    };

    //表示单步解析步骤的结果
    enum class ParseState{
        kNeedMore,  //数据不完整，等待更多数据
        kOk,        //本步完成
        kError      //报文非法
    };

    HttpContext();
    ~HttpContext();
    ParseResult ParseRequest(Buffer* buf);   //解析请求

    bool GotAll() const {return state_ == HttpRequestParseState::kGotAll;}
    void Reset();
    const HttpRequest& request() const {return request_;}

private:
    ParseState ParseRequestLine(Buffer*);    //解析请求行
    ParseState ParseHeaders(Buffer*);        //解析首部行
    ParseState ParseBody(Buffer*);           //解析信息主体
    bool ParseUrl(const std::string&);    //解析Url
    HttpRequest request_;
    HttpRequestParseState state_;
};


