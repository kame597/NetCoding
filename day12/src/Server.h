#pragma once
#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
class EventLoop;
class Socket;
class Channel;
class Acceptor;
class Connection;
class ThreadPool;

class Server{
private:
    EventLoop* mainReactor;
    std::vector<std::unique_ptr<EventLoop>> subReactors;
    std::unordered_map<int,std::unique_ptr<Connection>> connections;
    std::unique_ptr<Acceptor> acceptor;
    std::unique_ptr<ThreadPool> threadPool;
    std::mutex connMutex;  //我们需要锁来保护connections表，防止对多个线程对表同时读写
public:
    Server(EventLoop*);
    ~Server();

    //以下是绑定的两个回调函数，根据不同的socket执行不同功能       
    void newConnection(Socket* serv_sock);
    void deleteConnection(Socket* clnt_sock);
};