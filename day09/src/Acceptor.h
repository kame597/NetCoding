#pragma once
#include <memory>
#include <functional>

class EventLoop;
class Socket;
class InetAddress;
class Channel;
class Acceptor{
private:
    EventLoop* loop; //此处用裸指针，因为EventLoop生命周期最长
    std::unique_ptr<Socket> sock;
    std::unique_ptr<Channel> acceptChannel;
public:
    Acceptor(EventLoop* _loop);
    ~Acceptor();
    void acceptConnection();
    void setNewConnectionCallback(std::function<void(Socket*)>);

    std::function<void(Socket*)> newConnectionCallback;
};