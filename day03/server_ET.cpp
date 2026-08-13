/*
用epoll改写day02的服务器
当前版本的epoll是ET模式
*/
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>

void errif(bool condition, const char *errmsg){
    if(condition){
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}

int main(){
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);

    //将socket设置为非阻塞模式
    int flags = fcntl(listenfd, F_GETFL, 0);
    fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);


    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    bind(listenfd, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr));

    listen(listenfd, SOMAXCONN);

    int epfd = epoll_create1(0);
    errif(epfd == -1, "epoll实例创建失败");

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listenfd;

    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    errif(ret == -1, "epoll事件注册失败");

    struct epoll_event events[1024];

    while(true){
        int n_ready = epoll_wait(epfd, events, 1024, -1);
        errif(n_ready == -1, "epoll_wait failed!");

        for(int i = 0; i < n_ready; ++ i){
            int fd = events[i].data.fd;

            if(fd == listenfd){
                //ET模式
                while(true){
                    sockaddr_in clnt_addr{};
                    socklen_t len = sizeof(clnt_addr);
                    int clnt_fd = accept(listenfd, 
                                        reinterpret_cast<sockaddr*>(&clnt_addr),
                                        &len);
                    if(clnt_fd == -1){
                        if(errno == EAGAIN || errno == EWOULDBLOCK){
                            //EAGAIN说明不是错误，因此不需要触发errif
                            break;
                        }
                        errif(true, "accept error");
                    }
                    

                    int flags = fcntl(clnt_fd, F_GETFL, 0);
                    fcntl(clnt_fd, F_SETFL, flags | O_NONBLOCK); //将新的client设置为非阻塞

                    char ip_str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &clnt_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
                    std::cout << "新客户端: " << clnt_fd
                    << ", IP: " << ip_str
                    << ", 端口: " << ntohs(clnt_addr.sin_port) << std::endl;

                    struct epoll_event clnt_ev;
                    clnt_ev.events = EPOLLIN | EPOLLET;
                    clnt_ev.data.fd = clnt_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_fd, &clnt_ev);
                }
            }else{
                while(true){
                    char buf[1024];
                    memset(buf, 0, sizeof(buf));
                    ssize_t n = recv(fd, buf, sizeof(buf), 0);

                    if(n > 0){
                        std::cout << "收到来自客户端: " 
                        << fd << "的信息: " 
                        << buf << std::endl;
                        send(fd, buf, n, 0);
                    }else if(n == 0){
                        std::cout << "客户端: "
                        << fd << "链接关闭" << std::endl;
                        close(fd);
                        break;
                    }else {
                        if(errno == EAGAIN || errno == EWOULDBLOCK) break;
                        std::cout << "客户端: " << fd << "错误" << std::endl;
                        close(fd);
                        break;
                    }
                }
            }
        }
    }

    return 0;
}