#pragma once
#include <functional>
#include <memory>
#include "Buffer.h"

class EventLoop;
class Socket;
class Channel;
class Connection{
private:
    EventLoop* loop;
    std::unique_ptr<Socket> sock;           //客户端的socket
    std::unique_ptr<Channel> channel;       //客户端的channel
    std::function<void(Socket*)> deleteConnectionCallback;

    Buffer readBuffer;     //这个缓存是输入缓冲区，存储read的内容
    Buffer writeBuffer;    //该缓存是输出缓冲区，存储write的内容
public:
    Connection(EventLoop* _loop, Socket* _sock);
    ~Connection();

    void echo(int sockfd);
    void setDeleteConnectionCallback(std::function<void(Socket*)>);
    
};  