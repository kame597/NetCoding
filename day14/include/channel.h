#pragma once
#include <sys/epoll.h>
#include <functional>
#include "common.h"

class EventLoop;
class Channel{
public:
    DISALLOW_COPY_AND_MOVE(Channel);
    Channel(int _fd, EventLoop* _loop);
    ~Channel();

    void HandleEvent() const;   //处理事件
    void EnableRead();          //允许读
    void EnableWrite();         //允许写
    void EnableET();            //指定ET模式
    void DisableWrite();        //todo禁用写，待实现
  
    int fd()const { return fd_;}
    uint32_t listen_events()const { return listen_events_; }
    uint32_t ready_events()const { return ready_events_; }

    bool in_epoll()const { return in_epoll_; }
    void set_in_epoll(bool in = true){ in_epoll_ = in;}

    void set_ready_events(uint32_t ev){ ready_events_ = ev; }
    void set_read_callback(std::function<void()>const&);
    void set_write_callback(std::function<void()>const&);

private:
    EventLoop* loop_;    // 持有EventLoop指针，用于获取epoll来反向操作
    int fd_;             // 该channel包装的fd

    uint32_t listen_events_;    // 记录希望内核监视的事件类型
    uint32_t ready_events_;   // 记录epoll_wait返回时，这个fd的真正发生事件
    bool in_epoll_;       // 用于区分当前channel是否在epoll红黑树中，以此区分ADD、MOD和DEL

    std::function<void()> read_callback_;
    std::function<void()> write_callback_;
};