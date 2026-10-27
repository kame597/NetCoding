#include "tcp_server.h"
#include "event_loop.h"
#include "event_loop_thread_pool.h"
#include "channel.h"
#include "acceptor.h"
#include "tcp_connection.h"
#include "utils.h"
#include "current_thread.h"

#include <memory>
#include <utility>
#include <functional>
#include <unistd.h>
#include <iostream>
#include <cerrno>



TcpServer::TcpServer(EventLoop* loop, const char* ip, const int port):main_reactor_(loop),next_conn_id_(1){

    acceptor_ = std::make_unique<Acceptor>(main_reactor_, ip, port);
    std::function<void(int)> cb = std::bind(&TcpServer::HandleNewConnection, this, std::placeholders::_1);
    acceptor_->set_newconnection_callback(cb);

    thread_pool_ = std::make_unique<EventLoopThreadPool>(loop);
    set_thread_nums(std::thread::hardware_concurrency());
}

TcpServer::~TcpServer(){}

/*
*   @brief
*   开启tcp服务器，即开启主rector事件循环和线程池
*/
void TcpServer::Start(){
    thread_pool_->Start();
    main_reactor_->Loop();
}



void TcpServer::HandleNewConnection(int fd){
    errif(fd == -1, "[新建客户端]该socketfd无效, 无法创建连接");
    // int random = fd % sub_reactors_.size();

    EventLoop* sub_reactor_ = thread_pool_->next_loop();

    // 创建TcpConnection对象，交付给指定的sub_reactor_
    std::shared_ptr<TcpConnection> conn = std::make_shared<TcpConnection>(sub_reactor_, fd, next_conn_id_);

    // std::function<void(const std::shared_ptr<TcpConnection> &)> cb = std::bind(&TcpServer::HandleClose, this, std::placeholders::_1);
    conn->set_close_callback(std::bind(&TcpServer::HandleClose, this, std::placeholders::_1));

    conn->set_connection_callback(on_connection_);
    conn->set_message_callback(on_message_);
    connections_map_[fd] = conn;    // 此处因为用了共享指针，可以直接赋值

    // 更新连接的id
    ++ next_conn_id_;
    
    // 开始监听读事件
    conn->ConnectionEstablish();
}

/*
*   HandleClose
*   @brief 
*   先调用上层Http的删表操作，再调用自身层的延迟析构功能
*/
void TcpServer::HandleClose(const std::shared_ptr<TcpConnection> & conn){
    if(on_close_) on_close_(conn);
    main_reactor_->RunInLoop(std::bind(&TcpServer::HandleCloseInLoop, this, conn));
}

/*
*   @brief
*   具体的关闭处理，主要是从connections_map_表中删掉conn，并将析构任务投递到sub线程执行
*   投递到sub线程执行是为了延迟执行ConnectionDestruct，当前子Reactor的loop循环理应正在执行HandleEvent
*   执行完HandleEvent，才会执行在延时队列里的ConnectionDestruct，而ConnectionDestruct既可以清除channel，在函数返回后引用归零，还会清除TcpConnection
*/
void TcpServer::HandleCloseInLoop(const std::shared_ptr<TcpConnection> & conn){
    std::cout << current_thread::tid()  << 
    " TcpServer::HandleCloseInLoop - Remove connection id: " <<  conn->id() << " and fd: " << conn->fd() << std::endl;

    errif(connections_map_.find(conn->fd()) == connections_map_.end(), "[删除客户端]该socketfd无效, 无法删除");
    connections_map_.erase(conn->fd());

    conn->loop()->QueueInLoop(std::bind(&TcpConnection::ConnectionDestruct, conn));
}

void TcpServer::set_thread_nums(int thread_nums){
    thread_pool_->set_thread_nums(thread_nums);
}