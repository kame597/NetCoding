#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include <memory>
#include <utility>
#include <functional>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>

/*
Server的构造函数，绑定一个EventLoop
并且创建好服务器socket，完成地址绑定，监听，设置非阻塞
随后创建服务器Channel，配置回调函数
*/
Server::Server(EventLoop* _loop):loop(_loop){
    serv_sock = std::make_unique<Socket>();
    std::unique_ptr<InetAddress> serv_addr = std::make_unique<InetAddress>("127.0.0.1", 8888);
    serv_sock->bind(serv_addr.get());
    serv_sock->listen(1024);
    serv_sock->setNoBlocking();

    servChannel = std::make_unique<Channel>(loop, serv_sock->getFd());
    //TODO bind函数参数绑定
    std::function<void()> cb = std::bind(&Server::newConnection, this, serv_sock.get());
    servChannel->setCallback(cb);
    servChannel->enableReading();
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
    //TODO 参数绑定
    std::function<void()> cb = std::bind(&Server::handleReadEvent, this, clntFd);
    clntChannel->setCallback(cb);
    clntChannel->enableReading();

    Client c;
    c.sock = std::move(clnt_sock);
    c.ch = std::move(clntChannel);
    clients.emplace(clntFd, std::move(c));
}