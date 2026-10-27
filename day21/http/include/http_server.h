#pragma once
#include "common.h"
#include "http_connection.h"

#include "tcp_server.h"

#include <unordered_map>
#include <memory>
#include <mutex>
#include <functional>

/*
*   HttpServer的职责
*   1. 持有并启动TcpServer，由TcpServer负责Acceptor和线程池
*   2. 维护connection_map_，管理HttpConnection
*   3. 转发回调，将Tcp层回调内容，翻译成HttpConnection的操作
*   4. 保存业务回调并注入给HttpConnection，即用户（main）注册的业务逻辑
*/
class HttpServer{
public:
    DISALLOW_COPY_AND_MOVE(HttpServer);
    HttpServer(EventLoop* loop, const char* ip, const int port);
    ~HttpServer();

    void Start();
    void set_message_callback(HttpConnection::MessageCallback cb);

private:
    void OnConnection(const std::shared_ptr<TcpConnection>& conn);
    void OnMessage(const std::shared_ptr<TcpConnection>& conn, Buffer* buf);
    void OnClose(const std::shared_ptr<TcpConnection>& conn);

    std::unique_ptr<TcpServer> tcp_server_;
    std::unordered_map<uint64_t, std::shared_ptr<HttpConnection>> http_conns_;   //此处为什么也要用共享指针呢？ --同样是保护作用，防止在执行功能时提前析构导致悬垂访问
    std::mutex conns_mutex_;                            //保护connection_map_的跨线程读写
    HttpConnection::MessageCallback message_callback_;  //用于注册业务逻辑的回调函数
};