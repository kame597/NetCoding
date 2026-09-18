#include "event_loop_thread_pool.h"
#include "event_loop_thread.h"
#include "event_loop.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *loop):
    main_reactor_(loop), thread_nums_(0), next_(0){}

EventLoopThreadPool::~EventLoopThreadPool(){}

void EventLoopThreadPool::Start(){
    for(int i = 0; i < thread_nums_; ++ i){
        std::unique_ptr<EventLoopThread> event_loop_thread_ptr = std::make_unique<EventLoopThread>();
        threads_.emplace_back(std::move(event_loop_thread_ptr));
        loops_.emplace_back(threads_.back()->StartLoopThread());
    }
}

EventLoop *EventLoopThreadPool::next_loop(){
    EventLoop* ret = main_reactor_;
    if(!loops_.empty()){
        ret = loops_[next_ ++];
        if(next_ == static_cast<int>(loops_.size()))
            next_ = 0;
    }
    return ret;
}
