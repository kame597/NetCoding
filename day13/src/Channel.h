#pragma once
#include <sys/epoll.h>
#include <functional>

/*
该版本的Channel不碰Epoll的细节，只负责包装一个fd，事件，和回调
Epoll的操作由EventLoop负责
*/
class EventLoop;
class Channel{
private:
    EventLoop* loop;    // 持有EventLoop指针，用于获取epoll来反向操作
    int fd;             // 该channel包装的fd
    uint32_t events;    // 记录希望内核监视的事件类型
    uint32_t ready;   // 记录epoll_wait返回时，这个fd的真正发生事件
    bool inEpoll;       // 用于区分当前channel是否在epoll红黑树中，以此区分ADD、MOD和DEL

    std::function<void()> readCallback;
    std::function<void()> writeCallback;

public:
    Channel(EventLoop* _loop, int _fd);
    ~Channel();

    void handleEvent();
    void enableReading();
  
    int getFd(){ return fd; }
    uint32_t getEvents(){ return events; }
    uint32_t getReady(){ return ready; }
    bool getInEpoll(){ return inEpoll; }
    void setInEpoll(bool in = true){ inEpoll = in;}

    void setReady(uint32_t ev){ ready = ev; }
    void setReadCallback(std::function<void()>);
};