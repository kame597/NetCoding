#include "acceptor.h"
#include "event_loop.h"
#include "channel.h"
#include "utils.h"

#include "sys/fcntl.h"
#include "sys/socket.h"

#include "arpa/inet.h"

#include <unistd.h>
#include <assert.h>
#include <iostream>

/*
*   Acceptor的构造函数
*   创建服务器的sock，绑定地址，开启监听
*   创建对应的channel，绑定创建连接的回调函数，将其添加到epoll中
*/
Acceptor::Acceptor(EventLoop* loop, const char* ip, const int port)
    : loop_(loop), listenfd_(-1){
    Create();
    Bind(ip, port);
    Listen();
    listen_channel_ = std::make_unique<Channel>(listenfd_, loop_);
    std::function<void()> cb = std::bind(&Acceptor::AcceptConnection, this);
    listen_channel_->set_read_callback(cb);
    listen_channel_->EnableRead();
}

Acceptor::~Acceptor(){
}

void Acceptor::Create(){
    assert(listenfd_ == -1);
    //此处的SOCK_CLOEXEC是什么？---用于进程exec()替换为新程序时自动关闭fd，防止fd泄露给新程序
    listenfd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    errif(listenfd_ == -1, "监听socket创建失败");
}

void Acceptor::Bind(const char* ip, const int port){
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(port);
    errif((::bind(listenfd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 1),
             "监听socket地址绑定失败");
}

void Acceptor::Listen(){
    assert(listenfd_ != -1);
    errif(::listen(listenfd_, SOMAXCONN) == -1, 
            "监听socket监听失败");
}

/*
*   AcceptConnection函数
*   负责接收连接，此处用while循环，防止在就绪队列为空的情况下调用accept导致错误
*   先accept，如果成功，则根据clnt_fd创建连接socket，并将其移动到
*/
void Acceptor::AcceptConnection(){
    struct sockaddr_in client{};
    socklen_t client_addrLength = sizeof(client);
    assert(listenfd_ != -1);

    int clnt_fd = ::accept4(listenfd_, reinterpret_cast<sockaddr*>(&client), &client_addrLength, SOCK_CLOEXEC | SOCK_NONBLOCK);

    errif(clnt_fd == -1, "服务器接收客户端失败");

    if(new_connection_callback_){
        new_connection_callback_(clnt_fd);
    }
}

/*
*   set_newconnection_callback函数
*   将一个可调用对象赋给new_connection_
*   new_connection_callback_将在AcceptConnection中调用
*/
void Acceptor::set_newconnection_callback(std::function<void(int)>const& cb){
    new_connection_callback_ = std::move(cb);
}