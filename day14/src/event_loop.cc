#include "include/event_loop.h"
#include "include/epoller.h"
#include "include/channel.h"
#include <vector>


EventLoop::EventLoop(){
    poller_ = std::make_unique<Epoller>(0);
}

EventLoop::~EventLoop(){}

/*
loop函数，用于开始事件驱动
本质上就是原来的程序中调用`epoll_wait()`函数的死循环
*/
void EventLoop::Loop() const{
    while(true){
        for(auto active_ch : poller_->Poll()){
            active_ch->HandleEvent();
        }
    }
}

void EventLoop::UpdateChannel(Channel* channel) const {
    poller_->UpdateChannel(channel);
}

void EventLoop::DeleteChannel(Channel* channel) const {
    poller_->DeleteChannel(channel);
}

