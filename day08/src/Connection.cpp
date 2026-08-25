#include "Connection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>

/*
Connection构造函数
初始化EventLoop与监听sock
*/
Connection::Connection(EventLoop* _loop, Socket* _sock):
    loop(_loop), sock(_sock), channel(nullptr){
    
    channel = std::make_unique<Channel>(_loop, sock->getFd());
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->setCallback(cb);
    channel->enableReading();
}

Connection::~Connection(){
    //因为用了智能指针所以不需要手动释放😋
}

void Connection::echo(int sockfd){
    char buf[1024];
    while(true){
        memset(buf, 0, sizeof(buf));
        ssize_t readByets = read(sockfd, buf, sizeof(buf));

        if(readByets > 0){
            std::cout << "[服务器]收到来自客户端 " << sockfd 
            << " 的信息: " << buf << std::endl;
            write(sockfd, buf, readByets);
        }
        else if(readByets == -1 && errno == EINTR){
            std::cout << "[服务器]正常中断，继续读取" << std::endl;
            continue;
        }
        else if(readByets == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){
            std::cout << "[服务器]读取完成" << std::endl;
            break;
        }
        else if(readByets == 0){
            std::cout << "[服务器]客户端断开链接" << std::endl;
            deleteConnectionCallback(sock.get());
            break;
        }
    }
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> cb){
    deleteConnectionCallback = cb;
}