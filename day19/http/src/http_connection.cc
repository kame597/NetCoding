#include "http_connection.h"

HttpConnection::HttpConnection(const std::shared_ptr<TcpConnection> &conn):
    conn_(conn)
{}

HttpConnection::~HttpConnection()
{}

const std::shared_ptr<TcpConnection> &HttpConnection::tcp_conn() const{
    return conn_;
}

HttpContext &HttpConnection::context(){
    return context_;
}

int HttpConnection::keepalive_count() const{
    return keepalive_count_;
}

void HttpConnection::incr_keepalive(){
    ++ keepalive_count_;
}

bool HttpConnection::should_close() const{
    return should_close_;
}

void HttpConnection::set_should_close(bool status){
    should_close_ = status;
}