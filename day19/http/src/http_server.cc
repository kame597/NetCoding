#include "http_server.h"

HttpServer::HttpServer(EventLoop* loop, const char* ip, const int port){
    tcp_server_ = std::make_unique<TcpServer>(loop, ip, port);
    tcp_server_->set_connection_callback(
        std::bind(&HttpServer::OnConnection, this, std::placeholders::_1)
    );
    tcp_server_->set_message_callback(
        std::bind(&HttpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2)
    );
    tcp_server_->set_close_callback(
        std::bind(&HttpServer::OnClose, this, std::placeholders::_1)
    );
}

HttpServer::~HttpServer(){
}

/*
*   @brief
*   启动TcpServer，由TcpServer负责Acceptor和线程池
*/
void HttpServer::Start(){
    tcp_server_->Start();
}

/*
*   @brief
*   保存业务层设定的回调，后续注入给HttpConnection，即用户（main）注册的业务逻辑
*/
void HttpServer::set_message_callback(HttpConnection::MessageCallback cb){
    message_callback_ = cb;
}

/*
*   @brief
*   用于转发回调，将Tcp层的OnConnect回调转化为HttpConnection的相关操作
*   OnConnection用于添加新的HttpConnection
*/
void HttpServer::OnConnection(const std::shared_ptr<TcpConnection> &conn){
    std::shared_ptr<HttpConnection> http_conn = std::make_shared<HttpConnection>(conn);
    http_conn->set_message_callback(message_callback_);
    {
        std::lock_guard<std::mutex> lock(conns_mutex_);
        http_conns_[conn->id()] = http_conn;
    }
}

/*
*   @brief
*   用于转发回调，将Tcp层的OnConnect回调转化为HttpConnection的相关操作
*   OnMessage用于HttpConnection处理信息
*/
void HttpServer::OnMessage(const std::shared_ptr<TcpConnection> &conn, Buffer *buf){
    std::shared_ptr<HttpConnection> http_conn;
    {
        std::lock_guard<std::mutex> lock(conns_mutex_);
        auto it = http_conns_.find(conn->id());
        if(it == http_conns_.end()) return;
        http_conn = it->second;
    }
    http_conn->OnMessage(buf);
}

void HttpServer::OnClose(const std::shared_ptr<TcpConnection> &conn){
    {
        std::lock_guard<std::mutex> lock(conns_mutex_);
        auto it = http_conns_.find(conn->id());
        if(it != http_conns_.end())
            http_conns_.erase(it);
    }
}
