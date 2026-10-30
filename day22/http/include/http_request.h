#pragma once
#include <string>
#include <map>
#include "common.h"

/*
*   HttpRequest是一个DTO(Data Transfer Object)
*   即一个数据容器，只负责装载解析好的HTTP请求，不负责解析，也不负责处理
*/
class HttpRequest{
public:
    // 命名规范，常量或枚举值，用k + pascal风格
    enum class Method{
        kInvalid = 0,   // 枚举本质是整数，设定起始元素值为0，后续元素各自+1
        kGet,
        kPost,
        kHead,
        kPut,
        kDelete
    };

    enum class Version{
        kUnknown = 0,
        kHttp10,
        kHttp11
    };

    HttpRequest();
    ~HttpRequest();

    void set_version(const std::string& str);  //Http版本号
    Version version() const;
    std::string version_str() const;

    void set_method(const std::string& method);    //Http请求方法
    Method method() const;
    std::string method_str() const;

    void set_url(const std::string& url);   //Http请求的url
    const std::string& url() const;

    void set_request_params(const std::string& key, const std::string& val);    
    const std::map<std::string, std::string>& request_params() const;
    const std::string get_request_value(const std::string& key) const;

    void add_header(const std::string &field, const std::string& value);
    const std::map<std::string, std::string>& header() const;
    std::string get_header_value(const std::string& field) const;
    
    void set_body(const std::string str);
    const std::string& body() const;

    void Clear();

private:
    Method method_;
    std::string url_;
    std::map<std::string, std::string> request_params_;  //对应请求目标里的查询串和查询参数
    Version version_;
    std::map<std::string, std::string> header_;   //对应所有首部字段
    std::string body_;
};

/*
*   示例报文与成员对应关系：
*   POST /search?q=cpp&page=2 HTTP/1.1\r\n     ← 请求行（Request Line）
*   Host: example.com\r\n                      ← 头部字段（Header）
*   Content-Type: application/x-www-form-urlencoded\r\n
*   Content-Length: 13\r\n
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
