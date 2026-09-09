#include "include/tcp_server.h"
#include "include/event_loop.h"
#include "include/thread_pool.h"
#include "include/channel.h"
#include "include/acceptor.h"
#include "include/tcp_connection.h"
#include "include/utils.h"

#include <memory>
#include <utility>
#include <functional>
#include <unistd.h>
#include <iostream>
#include <cerrno>



TcpServer::TcpServer(const char* ip, const int port):next_conn_id_(1){
    main_reactor_ = std::make_unique<EventLoop>();

    acceptor_ = std::make_unique<Acceptor>(main_reactor_.get(), ip, port);
    std::function<void(int)> cb = std::bind(&TcpServer::HandleNewConnection, this, std::placeholders::_1);
    acceptor_->set_newconnection_callback(cb);

    int size = std::thread::hardware_concurrency();
    thread_pool_ = std::make_unique<ThreadPool>(size);
    
    for(int i = 0; i < size; ++ i){
        sub_reactors_.emplace_back(std::make_unique<EventLoop>());
    }
}

TcpServer::~TcpServer(){}

/*
*   @brief
*   创建子循环，插入线程池，开启主循环
*/
void TcpServer::Start(){
    for(size_t i = 0; i < sub_reactors_.size(); ++ i){
        thread_pool_->Add(&EventLoop::Loop, sub_reactors_[i].get());
    }
    main_reactor_->Loop();
}



void TcpServer::HandleNewConnection(int fd){
    errif(fd == -1, "[新建客户端]该socketfd无效, 无法创建连接");
    int random = fd % sub_reactors_.size();

    std::unique_ptr<TcpConnection> conn = std::make_unique<TcpConnection>(sub_reactors_[random].get(), fd, next_conn_id_);
    std::function<void(int)> cb = std::bind(&TcpServer::HandleClose, this, std::placeholders::_1);
    conn->set_close_callback(cb);

    conn->set_message_callback(on_message_);
    {
        std::lock_guard<std::mutex> lock(conns_map_mutex_);
        connections_map_[fd] = std::move(conn);
    }

    // 更新连接的id
    ++ next_conn_id_;
    if(next_conn_id_ == 1000)
        next_conn_id_ = 1;
}

void TcpServer::HandleClose(int fd){
    errif(connections_map_.find(fd) == connections_map_.end(), "[删除客户端]该socketfd无效, 无法删除");
    {
        std::lock_guard<std::mutex> lock(conns_map_mutex_);
        connections_map_.erase(fd); //因为我们用的智能指针存储，所以直接在容器里删除即可，不用管指针释放
    }
}