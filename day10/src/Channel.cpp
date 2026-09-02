#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop* _loop, int _fd):
    loop(_loop), fd(_fd), events(0), revents(0), inEpoll(false){}

Channel::~Channel(){

}

/*
handleEvent函数
将可调用对象callback扔给线程池，交给子线程执行
*/
void Channel::handleEvent(){
    loop->addTask(callback);
    //callback();  
}

/*
enableReading函数
将channel的events设为“数据可读”和“单次发送”，表明我们希望监听该channel上发生的读事件
并调用updateChannel函数，将channel封装的fd和events存入epoll中
*/
void Channel::enableReading(){
    events = EPOLLIN | EPOLLONESHOT;
    loop -> updateChannel(this);
}

int Channel::getFd(){
    return fd;
}

uint32_t Channel::getEvents(){
    return events;
}

uint32_t Channel::getRevent(){
    return revents;
}

bool Channel::getInEpoll(){
    return inEpoll;
}

void Channel::setInEpoll(){
    inEpoll = true;
}

void Channel::setEvent(uint32_t _ev){
    events = _ev;
}

void Channel::setRevent(uint32_t _ev){
    revents = _ev;
}

void Channel::setCallback(std::function<void()> _cb){
    callback = _cb;
}