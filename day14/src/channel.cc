#include "include/channel.h"
#include "include/event_loop.h"

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

    //TODO 这里的EPOLLRDHUP是啥？？？
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


void Channel::set_read_callback(std::function<void()>const& cb){
    read_callback_ = std::move(cb);
}

void Channel::set_write_callback(std::function<void()>const& cb){
    write_callback_ = std::move(cb);
}

