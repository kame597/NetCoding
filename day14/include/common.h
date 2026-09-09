#pragma once

class TcpServer;
class TcpConnection;
class Epoller;
class Acceptor;
class Channel;
class Buffer;
class ThreadPool;
class Socket;
class EventLoop;

/*
*   TODO这个宏定义的原理是啥呢？？？
*/

#define DISALLOW_COPY(cname)    \
    cname(const cname &) = delete;   \
    cname &operator=(const cname& ) = delete;

#define DISALLOW_MOVE(cname)    \
    cname(cname &&) = delete;   \
    cname &operator=(cname &&) = delete;

#define DISALLOW_COPY_AND_MOVE(cname)   \
    DISALLOW_COPY(cname);               \
    DISALLOW_MOVE(cname);
    