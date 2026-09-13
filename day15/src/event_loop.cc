#include "include/event_loop.h"
#include "include/epoller.h"
#include "include/channel.h"

#include <sys/eventfd.h>

#include <unistd.h>
#include <assert.h>

EventLoop::EventLoop(){
    poller_ = std::make_unique<Epoller>(0);
    /*
    *   eventfd是一个内核计数器，初始为0，包装为fd
    *   创建一个eventfd，第一个参数代表初始值为0，第二个参数为标志位
    */
    wakeup_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    wakeup_channel_ = std::make_unique<Channel>(wakeup_fd_, this);
    calling_functors_ = false;

    wakeup_channel_->set_read_callback(std::bind(&EventLoop::HandleWakeUp, this));
    wakeup_channel_->EnableRead();
}

EventLoop::~EventLoop(){}

void EventLoop::Loop(){     
    /*
    *   获取当前线程的tid并存储
    */
    tid_ = current_thread::tid();   
    while(true){
        for(auto active_ch : poller_->Poll(-1)){  //关键阻塞点：Poll可能引发阻塞，导致DoPendingFunctors无法执行
            active_ch->HandleEvent();
        }
        DoPendingFunctors();    // 执行完就绪任务后，处理运行队列待处理的任务
    }
}

void EventLoop::DoPendingFunctors(){
    calling_functors_ = true;
    std::vector < std::function<void()>> functors;
    {
        std::lock_guard lock(pending_mutex_);
        /*
        *   此处用swap是为了缩小锁的临界区间，将空容器和任务容器的内容交换
        *   如果不上锁，则一边执行，一边vector插入，产生数据竞争
        *   如果只上锁，直接循环执行，那会长时间持有锁，浪费时间
        *   因此：选择上锁，获得当前待处理任务，在外部执行
        */
        functors.swap(pending_functors_);   
    }
    for(const auto& func : functors)
        func();
    calling_functors_ = false;
}

void EventLoop::QueueInLoop(std::function<void()> func){
    {
        std::lock_guard lock(pending_mutex_);
        pending_functors_.push_back(func);
    }
    /*
    *   当有执行任务入队后，只要满足条件，就往eventfd写8字节的1，让eventfd从0变1，激活为可读状态
    *   条件说明：
    *   !IsInLoopThread()，表示投递者是其他线程，此时直接唤醒
    *   IsInLoopThread()且calling_functors_=false，意味着此时正在执行HandleEvent，那么pending_functors自然会执行，因此无需唤醒
    *   IsInLoopThread()且calling_functors=true，意味着此时正在执行DoPendingFunctors，此时需要唤醒来执行新任务
    */
    if(!IsInLoopThread() || calling_functors_){
        uint64_t write_one_byte = 1;
        ssize_t write_size = ::write(wakeup_fd_, &write_one_byte, sizeof(write_one_byte));
        assert(write_size == sizeof(write_one_byte));   //正常写的话，eventfd一定会写满8字节，返回字节数一定是8
    }
}

void EventLoop::RunInLoop(std::function<void()> func){
    if(IsInLoopThread())
        func();
    else
        QueueInLoop(func);
}

/*
*   HandleWakeUp
*   @brief 用于wakeup_channel的回调函数，读取掉8字节的1，计数器从1到0，变为不可读状态
*/
void EventLoop::HandleWakeUp(){
    uint64_t read_one_byte = 1;
    ssize_t read_size = ::read(wakeup_fd_, &read_one_byte, sizeof(read_one_byte));
    assert(read_size == sizeof(read_one_byte));
    return;
}

void EventLoop::UpdateChannel(Channel* channel) const {
    poller_->UpdateChannel(channel);
}

void EventLoop::DeleteChannel(Channel* channel) const {
    poller_->DeleteChannel(channel);
}

