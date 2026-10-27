#include "event_loop_thread_pool.h"
#include "event_loop_thread.h"
#include "event_loop.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *loop):
    main_reactor_(loop), thread_nums_(0), next_(0){}

EventLoopThreadPool::~EventLoopThreadPool(){}

/*
*   Start函数
*   创建指定数量的线程，并开启对应线程的事件循环
*/
void EventLoopThreadPool::Start(){
    for(int i = 0; i < thread_nums_; ++ i){
        std::unique_ptr<EventLoopThread> event_loop_thread_ptr = std::make_unique<EventLoopThread>();
        threads_.emplace_back(std::move(event_loop_thread_ptr));
        loops_.emplace_back(threads_.back()->StartLoopThread());
    }
}

/*
*   next_loop函数
*   用于返回下一个事件循环，用于轮询
*   每当来一个新连接，就取下一个子reactor
*/
EventLoop *EventLoopThreadPool::next_loop(){
    EventLoop* ret = main_reactor_;
    if(!loops_.empty()){
        ret = loops_[next_ ++];
        if(next_ == static_cast<int>(loops_.size()))
            next_ = 0;
    }
    return ret;
}
