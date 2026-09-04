#pragma once
#include <memory>
#include <functional>
#include <ThreadPool.h>
class Epoll;
class Channel;

/*
EventLoop类持有Epoll，是真正的反应堆
负责epoll_wait的循环和事件分发
同时持有线程池对象
*/
class EventLoop{
private:
    std::unique_ptr<Epoll> ep;
    std::unique_ptr<ThreadPool> threadPool;
    bool quit;
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void updateChannel(Channel*);

    template<typename F>
    void addTask(F&& func);
};

/*
addTask函数
用于调用ThreadPool的add函数，往线程池里加入新的任务
采用完美转发的方式，如果传入的func是一个右值，则移动构造
*/
template<typename F>
void EventLoop::addTask(F&& func){
    threadPool->add(std::forward<F>(func));
}