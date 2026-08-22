#include "Channel.h"
#include "Epoll.h"

Channel::Channel(Epoll* _ep, int _fd):
    ep(_ep), fd(_fd), events(0), revents(0), inEpoll(false){}

Channel::~Channel(){

}

/*
enableReading函数
将channel的events设为“数据可读”，表明我们希望监听该channel上发生的读事件
并调用updateChannel函数，将channel封装的fd和events存入epoll中
*/
void Channel::enableReading(){
    events = EPOLLIN;
    ep -> updateChannel(this);
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