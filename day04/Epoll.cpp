#include "Epoll.h"
#include "utils.h"
#include "Socket.h"
#include <unistd.h>
#include <cstring>

Epoll::Epoll(int flag){
    epfd = epoll_create1(flag);   
    errif(epfd == -1, "epoll实例创建失败");
    events = std::make_unique<epoll_event[]>(1024);
}

Epoll::~Epoll(){
    if(epfd != -1){
        close(epfd);
        epfd = -1;
    }
}

void Epoll::addFd(Socket* socket, uint32_t op){
    struct epoll_event ev{};
    ev.events = op;
    ev.data.fd = socket->getFd();
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, socket->getFd(), &ev);
    errif(ret == -1, "epoll事件注册失败");
}

std::vector<epoll_event> Epoll::poll(int timeout){
    std::vector<epoll_event> activeEvents;
    int n_ready = epoll_wait(epfd, events.get(), 1024, timeout);
    errif(n_ready == -1, "epoll等待事件失败");
    for(int i = 0; i < n_ready; ++ i){
        activeEvents.emplace_back(events[i]);
    }
    return activeEvents;
}
