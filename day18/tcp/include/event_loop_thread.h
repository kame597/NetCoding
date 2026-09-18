#pragma once
#include "common.h"

#include <mutex>
#include <thread>
#include <condition_variable>

class EventLoop;

class EventLoopThread{
public:
    DISALLOW_COPY_AND_MOVE(EventLoopThread);
    EventLoopThread();
    ~EventLoopThread();

    // 启动线程, 使EventLoop成为IO线程
    EventLoop* StartLoopThread();

private:
    void ThreadFunc();  //当前线程所运行的函数

    // 指向由子线程创建的的EventLoop，用于主线程和子线程的数据传递
    EventLoop* created_loop_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
};