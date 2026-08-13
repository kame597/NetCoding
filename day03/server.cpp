/*
用epoll改写day02的服务器
当前版本的epoll是LT模式，本质上相当于效率较高的poll
*/
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

void errif(bool condition, const char *errmsg){
    if(condition){ 
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}

int main(int args, char* argv){
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    bind(sockfd, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr));
    listen(sockfd, SOMAXCONN);

    // 创建epoll实例
    int epfd = epoll_create1(0); //epoll_create1是现代版本，其参数为flags标志位，传0代表无特殊标志
    errif(epfd == -1, "epoll_create1 failed");

    // 注册服务器fd与对应事件
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;

    /*为什么epoll_ctl里有传fd，还需要在event中设置fd? 
        --第三个参数是告知内核监听该fd，event是用户自定义数据，内核只负责存储，在epoll_wait时返回
    */
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev); 
    errif(ret == -1, "epoll_ctl failed");

    struct epoll_event events[1024];

    while(true){
        // 获取所有就绪的事件，等待时间为永久；返回就绪事件的数量
        int n_ready = epoll_wait(epfd, events, 1024, -1);

        for(int i = 0; i < n_ready; ++ i){
            int fd = events[i].data.fd;

            // 情况1：监听socket收到信息
            if(fd == sockfd){
                sockaddr_in clnt_addr{};
                socklen_t len = sizeof(clnt_addr);
                int clnt_fd = accept(sockfd, reinterpret_cast<sockaddr*>(&clnt_addr), &len);
                errif(clnt_fd == -1, "新客户端接收失败");

                char ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &clnt_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
                std::cout << "新客户端: " << clnt_fd << ", IP: " << ip_str << ", 端口: " << ntohs(clnt_addr.sin_port) <<  std::endl;

                //将新客户端fd注册到事件表
                struct epoll_event clnt_ev;
                clnt_ev.events = EPOLLIN;
                clnt_ev.data.fd = clnt_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_fd, &clnt_ev);
            }else{
                // 情况2：链接socket收到来自客户端信息
                char buf[1024];
                memset(buf, 0, sizeof(buf));
                ssize_t n = recv(fd, buf, sizeof(buf), 0);
                
                if(n > 0){
                    std::cout << "收到来自客户端: " << fd << "的信息: " << buf << std::endl;
                    send(fd, buf, n, 0);
                }else if(n == 0){
                    std::cout << "客户端: " << fd << "链接关闭" << std::endl;
                    close(fd); //close后内核会自动移除事件表的对应fd
                }else if(n == -1){
                    std::cout << "客户端: " << fd << "错误，已关闭" << std::endl;
                    close(fd);
                }
            }
        }
    }

    return 0;
}

