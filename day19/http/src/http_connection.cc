#include "http_connection.h"

HttpConnection::HttpConnection(const std::shared_ptr<TcpConnection> &conn):
    conn_(conn)
{}

HttpConnection::~HttpConnection()
{}

void HttpConnection::OnMessage(Buffer *buf){

}

void HttpConnection::OnClose(){

}

void HttpConnection::set_message_callback(MessageCallback cb){
    message_callback_ = std::move(cb);
}

const std::shared_ptr<TcpConnection> &HttpConnection::tcp_conn() const{
    return conn_;
}


int HttpConnection::keepalive_count() const{
    return keepalive_count_;
}



bool HttpConnection::should_close() const{
    return should_close_;
}

void HttpConnection::HandleRequest(const HttpRequest &req)
{
}


void HttpConnection::SendResponse(const HttpResponse &resp)
{
}

void HttpConnection::Reset()
{
}
