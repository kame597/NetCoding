#include "Epoll.h"
#include "utils.h"
#include "Socket.h"
#include "Channel.h"
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

// std::vector<epoll_event> Epoll::poll(int timeout){
//     std::vector<epoll_event> activeEvents;
//     int n_ready = epoll_wait(epfd, events.get(), 1024, timeout);
//     errif(n_ready == -1, "epoll等待事件失败");
//     for(int i = 0; i < n_ready; ++ i){
//         activeEvents.emplace_back(events[i]);
//     }
//     return activeEvents;
// }

std::vector<Channel*> Epoll::poll(int timeout){
    std::vector<Channel*> activeChannels;
    int nfds = epoll_wait(epfd, events.get(), 1024, timeout);
    errif(nfds == -1, "epoll_wait错误");
    for(int i = 0; i < nfds; ++ i){
        /*
        此处直接通过epoll上的data.ptr，返回channel指针本身，用于取回完整的channel
        */
        Channel* ch = reinterpret_cast<Channel*>(events[i].data.ptr); 
        ch->setRevent(events[i].events);   // 记录该channel下监听的events类型
        activeChannels.emplace_back(ch);
    }
    return activeChannels;
}

/*
updateChannel函数
获取一个channel，将事件的ptr绑定到channel，并根据该channel的事件类型，执行不同的操作，如添加和修改
*/
void Epoll::updateChannel(Channel* channel){
    int fd = channel->getFd();
    struct epoll_event ev{};
    ev.data.ptr = channel;
    ev.events = channel->getEvents();

    if(!channel->getInEpoll()){
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add错误");
        channel->setInEpoll();
    }
    else{
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll mod错误");
    }
}
