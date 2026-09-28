#pragma once
#include <string>
#include <map>

// HttpResponse 是一个DTO，其最终要被序列化处理成字符串
class HttpResponse{
public:
    enum class StatusCode{
        kUnknown = 0,
        k200Ok = 200,
        k204NoContent = 204,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k403Forbidden = 403,
        k404NotFound = 404,
        k405MethodNotAllowed = 405,
        k500InternalServerError = 500
    };

    HttpResponse();
    ~HttpResponse();

    void set_status_code(StatusCode);
    StatusCode status_code() const;
    std::string status_message() const; //用于返回状态码对应的短语，例如200对应ok

    void set_version(const std::string&);
    const std::string& version()const;

    void add_header(const std::string& field, const std::string& value);
    std::string get_header_value(const std::string& field) const;   // 注：返回临时变量时不可以用const &返回类型，会导致常引用绑定到已销毁的内存上
    const std::map<std::string, std::string>& header() const;

    void set_body(const std::string& body);
    const std::string& body() const;

    std::string ToString() const; //用于序列化响应报文，是HttpResponse的核心功能

    void Clear();

private:
    StatusCode status_code_;
    //不需要存储具体的状态信息，因为其可以通过status_code_推导而出
    std::string version_;
    std::map<std::string, std::string> header_;
    std::string body_;
};


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