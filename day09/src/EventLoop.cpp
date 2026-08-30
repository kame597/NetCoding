#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include <vector>

/*
EventLoop构造函数
配置了Epoll类，和一个quit状态位
*/
EventLoop::EventLoop(){
    ep = std::make_unique<Epoll>(0);
    quit = false;
}

EventLoop::~EventLoop(){
    //因为我们用了智能指针所以无需手动删除:)
}

/*
loop函数，用于开始事件驱动
本质上就是原来的程序中调用`epoll_wait()`函数的死循环
*/
void EventLoop::loop(){
    while(!quit){
        std::vector<Channel*> chs;
        chs = ep->poll();
        for(auto it = chs.begin(); it != chs.end(); ++ it){
            (*it)->handleEvent();
        }
    }
}

/*
updateChannel函数
用于调用Epoll的updateChannel函数
*/
void EventLoop::updateChannel(Channel* channel){
    ep->updateChannel(channel);
}