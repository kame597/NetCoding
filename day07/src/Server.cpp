#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Acceptor.h"
#include <memory>
#include <utility>
#include <functional>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>

/*
Server的构造函数，绑定一个EventLoop
并且创建好Acceptor，传入回调函数给Acceptor
*/
Server::Server(EventLoop* _loop):loop(_loop), acceptor(nullptr){
    acceptor = std::make_unique<Acceptor>(_loop);
    //监听socket已经由Acceptor管理，因此要由Acceptor调用；此处用bind是绑定Server的指针，占位符意味着参数需要在Acceptor中传入
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server(){

}

/*
handleReadEvent函数
回调函数，用于处理接受客户端的信息
*/
void Server::handleReadEvent(int clntFd){
    char buf[1024];
    while(true){
        memset(buf, 0, sizeof(buf));
        ssize_t readBytes = read(clntFd, buf, sizeof(buf));
        if(readBytes > 0){
            std::cout << "[服务器]从客户端 " << clntFd << " 读取信息: " << buf << std::endl;
            write(clntFd, buf, readBytes);
        }
        else if(readBytes == -1 && errno == EINTR){ //正常中断，继续读取
            continue;
        }
        else if(readBytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){ //读取结束，跳出循环
            std::cout << "[服务器]读取结束" << std::endl;
            break;
        }
        else if(readBytes == 0){
            std::cout << "[服务器]客户端断开连接" << std::endl;
            clients.erase(clntFd);
            break;
        }
    }
}

/*
newConnection函数
回调函数，用于创建新链接，并给新的客户端channel配置回调函数
*/
void Server::newConnection(Socket* serv_sock){
    std::unique_ptr<Socket> clnt_sock = std::make_unique<Socket>(serv_sock->accept());
    std::cout << "[服务器]新客户端接入," << "fd: " << clnt_sock->getFd() << std::endl;
    clnt_sock->setNoBlocking();

    int clntFd = clnt_sock->getFd();
    std::unique_ptr<Channel> clntChannel = std::make_unique<Channel>(loop, clntFd);
    std::function<void()> cb = std::bind(&Server::handleReadEvent, this, clntFd);
    clntChannel->setCallback(cb);
    clntChannel->enableReading();

    Client c;
    c.sock = std::move(clnt_sock);
    c.ch = std::move(clntChannel);
    clients.emplace(clntFd, std::move(c));
}