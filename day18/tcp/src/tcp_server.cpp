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
    SetThreadNums(std::thread::hardware_concurrency());
}

TcpServer::~TcpServer(){}

/*
*   @brief
*   封装避免线程操作，开启主循环和线程池
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

    std::function<void(const std::shared_ptr<TcpConnection> &)> cb = std::bind(&TcpServer::HandleClose, this, std::placeholders::_1);
    conn->set_close_callback(cb);

    conn->set_connect_callback(on_connect_);
    conn->set_message_callback(on_message_);
    connections_map_[fd] = conn;    // 此处因为用了共享指针，可以直接赋值

    // 更新连接的id
    ++ next_conn_id_;
    if(next_conn_id_ == 1000)
        next_conn_id_ = 1;
    
    // 开始监听读事件
    conn->ConnectionEstablish();
}

/*
*   HandleClose
*   @brief 
*   由main_reactor调用RunInLoop，用main_reactor的tid判断是否为main_reactor来执行erase操作，
*   main_reactor_会调用IsInLoop判断，其中tid_是main_reactor_的，
*   而current_thread::tid()得到的是sub_reactor_的tid
*/
void TcpServer::HandleClose(const std::shared_ptr<TcpConnection> & conn){
    std::cout << current_thread::tid() << "TcpServer::HandleClose" << std::endl;
    main_reactor_->RunInLoop(std::bind(&TcpServer::HandleCloseInLoop, this, conn));
}

void TcpServer::HandleCloseInLoop(const std::shared_ptr<TcpConnection> & conn){
    std::cout << current_thread::tid()  << 
    " TcpServer::HandleCloseInLoop - Remove connection id: " <<  conn->id() << " and fd: " << conn->fd() << std::endl;

    errif(connections_map_.find(conn->fd()) == connections_map_.end(), "[删除客户端]该socketfd无效, 无法删除");
    connections_map_.erase(conn->fd());

    conn->loop()->QueueInLoop(std::bind(&TcpConnection::ConnectionDestruct, conn));
}

void TcpServer::SetThreadNums(int thread_nums){
    thread_pool_->set_thread_nums(thread_nums);
}