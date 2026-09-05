#include "Server.h"
#include "Socket.h"
#include "EventLoop.h"
#include "ThreadPool.h"
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
#include <thread>



/*
Server的构造函数，绑定一个EventLoop
并且创建好Acceptor，传入回调函数给Acceptor
*/
Server::Server(EventLoop* _loop):mainReactor(_loop), acceptor(nullptr){
    acceptor = std::make_unique<Acceptor>(_loop);
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);

    int size = std::thread::hardware_concurrency();
    threadPool = std::make_unique<ThreadPool>(size);
    
    for(int i = 0; i < size; ++ i){
        subReactors.emplace_back(std::make_unique<EventLoop>());
        threadPool->add(&EventLoop::loop, subReactors[i].get());
    }
}

Server::~Server(){
    connections.clear();
}


/*
*   @biref 创建一个新的TCP链接，绑定到一个随机的从属Reactor上
*   @param 客户端的socket
*/
void Server::newConnection(Socket* sock){
    int random = sock->getFd() % subReactors.size();
    /*
    *   此处注意到，新建的链接是绑定到了一个随机的subReactor的EventLoop上
    *   可见，新建的链接都是由从属Reactor负责执行的
    */
    std::unique_ptr<Connection> conn = std::make_unique<Connection>(subReactors[random].get(), sock);
    std::function<void(Socket*)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    conn->setDeleteConnectionCallback(cb);
    {
        std::lock_guard<std::mutex> lock(connMutex);
        connections[sock->getFd()] = std::move(conn);
    }
    
}

void Server::deleteConnection(Socket* sock){
    {
        std::lock_guard<std::mutex> lock(connMutex);
        connections.erase(sock->getFd()); //因为我们用的智能指针存储，所以直接在容器里删除即可，不用管指针释放
    }
    
}