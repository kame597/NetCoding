#pragma once
#include <functional>
#include <memory>

class EventLoop;
class Socket;
class Channel;
class Connection{
private:
    EventLoop* loop;
    std::unique_ptr<Socket> sock;           //客户端的socket
    std::unique_ptr<Channel> channel;       //客户端的channel
    std::function<void(Socket*)> deleteConnectionCallback;
public:
    Connection(EventLoop* _loop, Socket* _sock);
    ~Connection();

    void echo(int sockfd);
    void setDeleteConnectionCallback(std::function<void(Socket*)>);
};  