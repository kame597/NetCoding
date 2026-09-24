#include "http_request.h"

HttpRequest::HttpRequest():method_(Method::kInvalid), version_(Version::kUnknown){

}

HttpRequest::~HttpRequest(){}

void HttpRequest::set_version(const std::string &ver){
    if(ver == "HTTP/1.0")    version_ = Version::kHttp10;
    else if(ver == "HTTP/1.1")   version_ = Version::kHttp11;
    else    version_ = Version::kUnknown;
}

HttpRequest::Version HttpRequest::version() const{
    return version_;
}

std::string HttpRequest::version_str() const
{
    switch (version_)
    {
    case Version::kHttp10: return "HTTP/1.0";
    case Version::kHttp11: return "HTTP/1.1";
    default:    return "UNKNOWN";
    }
}

void HttpRequest::set_method(const std::string &method){
    if(method == "GET")    method_ = Method::kGet;
    else if(method == "POST")   method_ = Method::kPost;
    else if(method == "HEAD")   method_ = Method::kHead;
    else if(method == "PUT")   method_ = Method::kPut; 
    else if(method == "DELETE")    method_ = Method::kDelete;
    else method_ = Method::kInvalid;
}

HttpRequest::Method HttpRequest::method() const{
    return method_;
}

std::string HttpRequest::method_str() const{
    switch (method_){
    case Method::kGet:  return "GET";
    case Method::kDelete:  return "DELETE";
    case Method::kHead:  return "HEAD";
    case Method::kPost:  return "POST";
    case Method::kPut:  return "PUT";
    default:  return "INVALID";
    }
}

void HttpRequest::set_url(const std::string &url){
    url_ = url;
}

const std::string &HttpRequest::url() const{
    return url_;
}

void HttpRequest::set_request_params(const std::string &key, const std::string &val){
    request_params_[key] = val;
}

const std::map<std::string, std::string> & HttpRequest::request_params() const{
    return request_params_;
}

const std::string HttpRequest::get_request_value(const std::string &key) const{
    std::string ret;
    auto it = request_params_.find(key);
    return it == request_params_.end() ? ret : it->second;
}

/*
*   @brief
*   将field转化为小写后存储首部行数据
*/
void HttpRequest::add_header(const std::string &field, const std::string &value){
    std::string lower = field;  //通过副本的方式绕过const限定
    for(auto& c : lower)
        c = std::tolower(static_cast<unsigned char>(c));    //用unsigned char映射的理由：char大多有符号，如果传入的是非ASCII码，比如UTF-8，传进去就会变成负数
    header_[lower] = value;
}

const std::map<std::string, std::string> &HttpRequest::header() const{
    return header_;
}

std::string HttpRequest::get_header_value(const std::string &field) const{
    std::string lower = field;
    for(auto& c : lower)
        c = std::tolower(static_cast<unsigned char>(c));
    std::string ret;
    auto it = header_.find(lower);
    return it == header_.end() ? ret : it->second; 
}

void HttpRequest::set_body(const std::string body){
    body_ = std::move(body);
}

const std::string &HttpRequest::body() const{
    return body_;
}

void HttpRequest::Clear(){
    method_ = Method::kInvalid;
    version_ = Version::kUnknown;
    url_.clear();
    request_params_.clear();
    header_.clear();
    body_.clear();
}
