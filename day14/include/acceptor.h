#pragma once
#include "common.h"
#include <memory>
#include <functional>

class EventLoop;
class Channel;

class Acceptor{
public:
    DISALLOW_COPY_AND_MOVE(Acceptor);                               //禁止拷贝和移动的宏定义
    Acceptor(EventLoop* _loop, const char* ip, const int port);     //修改构造函数，采用直接传入ip和端口的方式 
    ~Acceptor();

    void set_newconnection_callback(std::function<void(int)>const& func);

    // 创建Socket
    void Create();

    // Socket与IP地址绑定
    void Bind(const char* ip, const int port);

    // 监听socket
    void Listen();

    // 接收链接
    void AcceptConnection();
private:
    EventLoop* loop_;    //此处用裸指针，因为EventLoop生命周期最长
    int listenfd_;       // 监听socket的fd
    std::unique_ptr<Channel> listen_channel_;
    std::function<void(int)> new_connection_callback_;
};