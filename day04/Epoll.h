#pragma once
#include <sys/epoll.h>
#include <vector>
#include <memory>

class Socket;
class Epoll{
private:
    int epfd = -1;
    std::unique_ptr<epoll_event[]> events;
public:
    Epoll() = default;
    Epoll(int flag);
    ~Epoll();

    void addFd(Socket*, uint32_t op);
    std::vector<epoll_event> poll(int timeout = -1);
};