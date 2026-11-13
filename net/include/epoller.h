#pragma once
#include "common.h"

#include <sys/epoll.h>
#include <vector>
#include <memory>

class Channel;
class Epoller{
public:
    DISALLOW_COPY_AND_MOVE(Epoller);

    Epoller() = default;
    Epoller(int flag);
    ~Epoller();

    void UpdateChannel(Channel* ch) const;
    void DeleteChannel(Channel* ch) const;

    // 返回调用完epoll_wait的通道事件
    std::vector<Channel*> Poll(int timeout = -1) const;
private:
    int epfd_ = -1;
    std::unique_ptr<epoll_event[]> events_;
};