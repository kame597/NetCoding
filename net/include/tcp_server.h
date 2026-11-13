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
class EventLoopThreadPool;
class TcpServer{
public:
    using ConnectionCallback = std::function<void (const std::shared_ptr<TcpConnection>)>;
    using MessageCallback = std::function<void (const std::shared_ptr<TcpConnection>, Buffer* buf)>;
    using CloseCallback = std::function<void (const std::shared_ptr<TcpConnection>)>;

    DISALLOW_COPY_AND_MOVE(TcpServer);
    TcpServer(EventLoop* loop, const char* ip, const int port);
    ~TcpServer();

    void Start();

    void set_connection_callback(ConnectionCallback cb){ on_connection_ = std::move(cb);}
    void set_message_callback(MessageCallback cb){ on_message_ = std::move(cb); }
    void set_close_callback(CloseCallback cb){ on_close_ = std::move(cb);}

    void HandleClose(const std::shared_ptr<TcpConnection> &);

    //  这是一层额外的封装，以保证erase操作是由main_reactor_来操作的
    void HandleCloseInLoop(const std::shared_ptr<TcpConnection> &);

    void HandleNewConnection(int fd);

    void set_thread_nums(int thread_nums);
    void set_conn_timeout_(int timeout);
    
private:
    EventLoop* main_reactor_;
    uint64_t next_conn_id_;

    std::unordered_map<int,std::shared_ptr<TcpConnection>> connections_map_;
    std::unique_ptr<Acceptor> acceptor_;

    std::unique_ptr<EventLoopThreadPool> thread_pool_;

    ConnectionCallback on_connection_;
    MessageCallback on_message_;
    CloseCallback on_close_;
    std::mutex conns_map_mutex_;  //我们需要锁来保护connections表，防止对多个线程对表同时读写

    int conn_timeout_ = 60; //默认连接超时为60s
};