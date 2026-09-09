#pragma once
#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
#include <functional>
#include "common.h"
class EventLoop;
class Acceptor;
class TcpConnection;
class ThreadPool;
class TcpServer{
public:
    DISALLOW_COPY_AND_MOVE(TcpServer);
    TcpServer(const char* ip, const int port);
    ~TcpServer();

    void Start();   //开启服务器

    void set_connection_callback(std::function<void(TcpConnection*)>const& cb){ on_connect_ = cb;}
    void set_message_callback(std::function<void(TcpConnection*)>const& cb){ on_message_ = cb; }

    void HandleClose(int fd);
    void HandleNewConnection(int fd);
    
private:
    std::unique_ptr<EventLoop> main_reactor_;
    int next_conn_id_;

    std::vector<std::unique_ptr<EventLoop>> sub_reactors_;
    std::unordered_map<int,std::unique_ptr<TcpConnection>> connections_map_;
    std::unique_ptr<Acceptor> acceptor_;

    std::unique_ptr<ThreadPool> thread_pool_;

    std::function<void(TcpConnection*)> on_connect_;
    std::function<void(TcpConnection*)> on_message_;
    std::mutex conns_map_mutex_;  //我们需要锁来保护connections表，防止对多个线程对表同时读写
};