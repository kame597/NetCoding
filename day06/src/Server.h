#pragma once
#include <unordered_map>
#include <memory>
class EventLoop;
class Socket;
class Channel;

struct Client{
    std::unique_ptr<Socket> sock;
    std::unique_ptr<Channel> ch;
};

class Server{
private:
    EventLoop* loop;
    std::unordered_map<int,Client> clients;
    std::unique_ptr<Socket> serv_sock;
    std::unique_ptr<Channel> servChannel;
public:
    Server(EventLoop*);
    ~Server();

    //以下是绑定的两个回调函数，根据不同的socket执行不同功能 //TODO
    void handleReadEvent(int);            
    void newConnection(Socket* serv_sock);
};