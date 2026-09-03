#pragma once
#include <unordered_map>
#include <memory>
#include <mutex>
class EventLoop;
class Socket;
class Channel;
class Acceptor;
class Connection;

class Server{
private:
    EventLoop* loop; //注：此处使用普通指针，是因为生命周期上，EventLoop是最长的
    std::unordered_map<int,std::unique_ptr<Connection>> connections;
    std::unique_ptr<Acceptor> acceptor;
    std::mutex connMutex;  //我们需要锁来保护connections表，防止对多个线程对表同时读写
public:
    Server(EventLoop*);
    ~Server();

    //以下是绑定的两个回调函数，根据不同的socket执行不同功能       
    void newConnection(Socket* serv_sock);
    void deleteConnection(Socket* clnt_sock);
};