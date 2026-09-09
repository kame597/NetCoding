#include "include/epoller.h"
#include "include/utils.h"
#include "include/channel.h"
#include <unistd.h>
#include <cstring>

Epoller::Epoller(int flag){
    epfd_ = epoll_create1(flag);   
    errif(epfd_ == -1, "epoll实例创建失败");
    events_ = std::make_unique<epoll_event[]>(1024);
}

//todo 这个析构也是必要的吗？？？
Epoller::~Epoller(){
    if(epfd_ != -1){
        ::close(epfd_);
        epfd_ = -1;
    }
}

/*
*   Poll函数
*   通过epoll_event[]的就绪event的data.ptr，得到对应的完整Channel
*   并记录下该channel监听的events类型，赋给Revent属性
*   返回就绪的Channel的集合
*/
std::vector<Channel*> Epoller::Poll(int timeout) const{
    std::vector<Channel*> active_channels;
    int nfds = epoll_wait(epfd_, events_.get(), 1024, timeout);
    errif(nfds == -1, "epoll_wait错误");

    for(int i = 0; i < nfds; ++ i){
        /*
        此处直接通过epoll上的data.ptr，返回channel指针本身，用于取回完整的channel
        */
        Channel* ch = reinterpret_cast<Channel*>(events_[i].data.ptr); 
        ch->set_ready_events(events_[i].events);   // 记录该channel下监听的events类型
        active_channels.emplace_back(ch);
    }
    return active_channels;
}

/*
*   UpdateChannel函数
*   获取一个channel，将事件的ptr绑定到channel，并根据该channel的事件类型，执行不同的操作，如添加和修改
*/
void Epoller::UpdateChannel(Channel* channel) const {
    int sockfd = channel->fd();
    struct epoll_event ev{};
    ev.data.ptr = channel;
    ev.events = channel->listen_events();

    if(!channel->in_epoll()){
        errif(epoll_ctl(epfd_, EPOLL_CTL_ADD, sockfd, &ev) == -1, "epoll add错误");
        channel->set_in_epoll();
    }
    else{
        errif(epoll_ctl(epfd_, EPOLL_CTL_MOD, sockfd, &ev) == -1, "epoll mod错误");
    }
}

/*
*   DeleteChannel函数
*/
void Epoller::DeleteChannel(Channel* channel) const{
    int sockfd = channel->fd();
    //todo 为什么这里可以不传epoll_events指针？？？
    errif((epoll_ctl(epfd_, EPOLL_CTL_DEL, sockfd, nullptr) == -1), "epoll del错误");
    channel->set_in_epoll(false);
}
