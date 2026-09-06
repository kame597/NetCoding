#pragma once
#include <sys/epoll.h>
#include <vector>
#include <memory>

class Socket;
class Channel;
class Epoll{
private:
    int epfd = -1;
    std::unique_ptr<epoll_event[]> events;
public:
    Epoll() = default;
    Epoll(int flag);
    ~Epoll();

    void updateChannel(Channel*);
    //std::vector<epoll_event> poll(int timeout = -1);
    std::vector<Channel*> poll(int timeout = -1);
};