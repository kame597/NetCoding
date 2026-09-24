#pragma once
#include "common.h"
#include "current_thread.h"

#include <sys/types.h>

#include <memory>
#include <functional>
#include <vector>
#include <mutex>

class Epoller;
class Channel;

class EventLoop{
public:
    DISALLOW_COPY_AND_MOVE(EventLoop);
    EventLoop();
    ~EventLoop();

    void Loop();
    void UpdateChannel(Channel*) const;
    void DeleteChannel(Channel*) const;

    //  运行队列中待处理的任务
    void DoPendingFunctors();

    //  入队，等 loop 统一执行
    void QueueInLoop(std::function<void()> func);

    //  在loop线程里运行
    void RunInLoop(std::function<void()> func);

    bool IsInLoopThread(){ return tid_ == current_thread::tid(); }

    // 响应唤醒
    void HandleWakeUp();

private:
    std::unique_ptr<Epoller> poller_;
    std::vector<std::function<void()>> pending_functors_;    //待处理任务队列
    std::mutex pending_mutex_;

    int wakeup_fd_;                            // 用于激活pending_functors执行的fd
    std::unique_ptr<Channel> wakeup_channel_;  // 用于激活pending_functors执行的channel

    bool calling_functors_;
    pid_t tid_;     //  存储当前线程的tid
};