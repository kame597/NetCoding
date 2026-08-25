#include "Acceptor.h"
#include "EventLoop.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
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
    acceptChannel->setCallback(cb);
    acceptChannel->enableReading();
}

Acceptor::~Acceptor(){
    //因为用了智能指针所以不需要手动销毁 :)
}

/*
acceptConnection函数

*/
void Acceptor::acceptConnection(){
    std::unique_ptr<Socket> clnt_sock = std::make_unique<Socket>(sock->accept());
    std::cout << "[服务器]接受客户端链接！" << "fd: " << clnt_sock->getFd() << std::endl;
    clnt_sock->setNoBlocking();
    newConnectionCallback(clnt_sock.release());
}

/*
setNewConnectionCallback函数
将一个可调用对象对newConnectionCallback赋值
newConnectionCallback将在acceptConnection中调用
*/
void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> _cb){
    newConnectionCallback = _cb;
}