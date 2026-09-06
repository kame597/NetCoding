#include "Acceptor.h"
#include "EventLoop.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "utils.h"
#include <iostream>

/*
Acceptor的构造函数
创建服务器的sock，并绑定EventLoop
*/
Acceptor::Acceptor(EventLoop* _loop) : loop(_loop){
    sock = std::make_unique<Socket>();
    std::unique_ptr<InetAddress> addr = std::make_unique<InetAddress>("127.0.0.1", 8888);
    sock->bind(addr.get());
    sock->listen(1024);
    sock->setNoBlocking();

    acceptChannel = std::make_unique<Channel>(_loop, sock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel->setReadCallback(cb);
    acceptChannel->enableReading();
}

Acceptor::~Acceptor(){
    //因为用了智能指针所以不需要手动销毁 :)
}

/*
acceptConnection函数
负责接收连接，此处用while循环，防止在就绪队列为空的情况下调用accept导致错误
先accept，如果成功，则根据clnt_fd创建连接socket，并将其移动到
*/
void Acceptor::acceptConnection(){
    while(true){
        int clnt_fd = sock -> accept();
        if(clnt_fd == -1){
            //注：此处如果accept调用，而已完成连接队列为空，就会导致errno为EAGAIN
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                break;
            }
            errif(true, "客户端连接创建失败");
        }
        //每成功accept一次，就立刻处理这个连接
        std::unique_ptr<Socket> clnt_sock = std::make_unique<Socket>(clnt_fd);
        std::cout << "[服务器]接受客户端链接！" << "fd: " << clnt_sock->getFd() << std::endl;
        clnt_sock->setNoBlocking();
        newConnectionCallback(clnt_sock.release());
    }
    acceptChannel->enableReading();
}

/*
setNewConnectionCallback函数
将一个可调用对象对newConnectionCallback赋值
newConnectionCallback将在acceptConnection中调用
*/
void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> _cb){
    newConnectionCallback = _cb;
}