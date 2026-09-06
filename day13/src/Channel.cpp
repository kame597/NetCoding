#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop* _loop, int _fd):
    loop(_loop), fd(_fd), events(0), ready(0), inEpoll(false){}

Channel::~Channel(){

}

/*
*   handleEvent函数
*   将可调用对象callback扔给线程池，交给子线程执行
*   如果是Acceptor，则直接调用不走线程池
*/
void Channel::handleEvent(){
    if(ready & (EPOLLIN | EPOLLPRI)){
        readCallback();
    }
    if(ready & EPOLLOUT){
        writeCallback();
    }
}

/*
*   enableReading函数
*   将channel的events设为“数据可读”和“单次发送”，表明我们希望监听该channel上发生的读事件
*   并调用updateChannel函数，将channel封装的fd和events存入epoll中
*/
void Channel::enableReading(){
    events |= EPOLLIN | EPOLLONESHOT | EPOLLPRI;
    loop -> updateChannel(this);
}

void Channel::setReadCallback(std::function<void()> cb){
    readCallback = std::move(cb);
}
