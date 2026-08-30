#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Acceptor.h"
#include "Connection.h"
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
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server(){

}


/*
newConnection函数
参数：sock表示客户端的socket
*/
void Server::newConnection(Socket* sock){
    std::unique_ptr<Connection> conn = std::make_unique<Connection>(loop, sock);
    std::function<void(Socket*)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    conn->setDeleteConnectionCallback(cb);
    connections[sock->getFd()] = std::move(conn);
}

void Server::deleteConnection(Socket* sock){
    connections.erase(sock->getFd()); //因为我们用的智能指针存储，所以直接在容器里删除即可，不用管指针释放
}