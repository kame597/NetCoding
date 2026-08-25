#pragma once
#include <memory>
class Epoll;
class Channel;

/*
EventLoop类持有Epoll，是真正的反应堆
负责epoll_wait的循环和事件分发
*/
class EventLoop{
private:
    std::unique_ptr<Epoll> ep;
    bool quit;
public:
    EventLoop();
    ~EventLoop();
    void loop();
    void updateChannel(Channel*);
};