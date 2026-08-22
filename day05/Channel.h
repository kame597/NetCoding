#pragma once
#include <sys/epoll.h>

/*
说明：
uint32_t是固定32位无符号整数，用于保证各个平台的移植性
channel本质上是封装fd和epoll的事件的对应关系
*/
class Epoll;
class Channel{
private:
    Epoll *ep;          // 用于channel反向操作Epoll
    int fd;             // 该channel包装的fd
    uint32_t events;    // 记录希望内核监视的事件类型
    uint32_t revents;   // 记录epoll_wait返回时，这个fd的真正发生事件
    bool inEpoll;       // 用于区分当前channel是否在epoll红黑树中，以此区分ADD、MOD和DEL

public:
  Channel(Epoll* ep, int fd);
  ~Channel();

  void enableReading();
  
  int getFd();
  uint32_t getEvents();
  uint32_t getRevent();
  bool getInEpoll();
  void setInEpoll();

  void setEvent(uint32_t);
  void setRevent(uint32_t);
  
};