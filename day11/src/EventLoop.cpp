#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include "ThreadPool.h"
#include <vector>

/*
EventLoop构造函数
配置了Epoll类，和一个quit状态位
*/
EventLoop::EventLoop(){
    ep = std::make_unique<Epoll>(0);
    threadPool = std::make_unique<ThreadPool>();
    quit = false;
}

EventLoop::~EventLoop(){
    //因为我们用了智能指针所以无需手动删除epoll和线程池:)
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

/*
addTask函数
用于调用ThreadPool的add函数，往线程池里加入新的任务
*/
void EventLoop::addTask(std::function<void()> func){
    threadPool->add(func);
}