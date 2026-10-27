#pragma once
#include "common.h"

#include <memory>
#include <vector>
#include <thread>

class EventLoop;
class EventLoopThread;
class EventLoopThreadPool{
public:
    DISALLOW_COPY_AND_MOVE(EventLoopThreadPool);
    EventLoopThreadPool(EventLoop*);
    ~EventLoopThreadPool();

    // 设置创建线程的数量，必须要在Start之前调用
    void set_thread_nums(int thread_nums){thread_nums_ = thread_nums;}

    void Start();

    EventLoop* next_loop();

private:
    EventLoop* main_reactor_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;

    int thread_nums_;
    int next_;
};