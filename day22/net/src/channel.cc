#include "channel.h"
#include "event_loop.h"

#include <unistd.h>

Channel::Channel(int fd, EventLoop* loop):
    loop_(loop), fd_(fd), listen_events_(0), ready_events_(0), in_epoll_(false){};

//由channel来负责fd的关闭
Channel::~Channel(){
    if(fd_ != -1){
        close(fd_);
        fd_ = -1;
    }
}

/*
*   handleEvent函数
*/
void Channel::HandleEvent() const {
    if(tied_){
        /*
        *   lock用于把weak_ptr提升为shared_ptr，引用计数+1
        *   用于保护TcpConnection对象在HandleEvent回调期间不被析构掉
        */
        std::shared_ptr<void> guard = tie_.lock();  
        HandleEventWithGuard();
    }else{
        HandleEventWithGuard();
    }
}

/*
*   Tie函数
*   @param ptr，表示TcpConnection的指针
*   @brief 绑定一个TcpConnection的指针，用于增加引用计数，实现延迟删除
*/
void Channel::Tie(const std::shared_ptr<void> &ptr){
    tied_ = true;
    tie_ = ptr;
}

/*
*   HandleEventWithGuard函数
*   根据当前channel的事件类型执行读回调或写回调
*   回调函数由TcpConnection注册
*/
void Channel::HandleEventWithGuard() const{
    if(ready_events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){
        if(read_callback_)  read_callback_();
    }
    if(ready_events_ & EPOLLOUT){
        if(write_callback_) write_callback_();
    }
}

/*
*   EnableRead函数
*   将channel的events设为“数据可读”，表明我们希望监听该channel上发生的读事件
*   并调用updateChannel函数，将channel封装的fd和events存入epoll中
*/
void Channel::EnableRead(){
    listen_events_ |= (EPOLLIN | EPOLLPRI);
    loop_ -> UpdateChannel(this);
}

void Channel::EnableWrite(){
    listen_events_ |= EPOLLOUT;
    loop_->UpdateChannel(this);
}

void Channel::EnableET(){
    listen_events_ |= EPOLLET;
    loop_->UpdateChannel(this);
}

void Channel::DisableRead(){
    listen_events_ &= ~(EPOLLIN | EPOLLPRI);
    loop_->UpdateChannel(this);
}

void Channel::set_read_callback(std::function<void()>const& cb){
    read_callback_ = std::move(cb);
}

void Channel::set_write_callback(std::function<void()>const& cb){
    write_callback_ = std::move(cb);
}

