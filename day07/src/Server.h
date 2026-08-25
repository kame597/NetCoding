#pragma once
#include <unordered_map>
#include <memory>
class EventLoop;
class Socket;
class Channel;
class Acceptor;

struct Client{
    std::unique_ptr<Socket> sock;
    std::unique_ptr<Channel> ch;
};

class Server{
private:
    EventLoop* loop; //注：此处使用普通指针，是因为生命周期上，EventLoop是最长的
    std::unordered_map<int,Client> clients;
    std::unique_ptr<Acceptor> acceptor;
public:
    Server(EventLoop*);
    ~Server();

    //以下是绑定的两个回调函数，根据不同的socket执行不同功能
    void handleReadEvent(int);            
    void newConnection(Socket* serv_sock);
};