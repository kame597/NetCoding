#include "InetAddress.h"
#include "Socket.h"
#include "Epoll.h"
#include "utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <utility> //存放了标准库move函数，用于获得绑定到左值上的右值引用
#include <cstring>
#include <iostream>

int main(){
    std::unique_ptr<Socket> serv = std::make_unique<Socket>();
    std::unique_ptr<InetAddress> serv_addr = std::make_unique<InetAddress>("127.0.0.1", 8888);
    serv->bind(serv_addr.get());
    serv->listen(SOMAXCONN);
    serv->setNoBlocking(); //设置为非阻塞模式

    std::unique_ptr<Epoll> epoll = std::make_unique<Epoll>(0);
    epoll->addFd(serv.get(), EPOLLIN); //监听socket使用LT模式

    std::unordered_map<int, std::unique_ptr<Socket>> clients;

    while(true){
        std::vector<epoll_event> events = epoll->poll();
        for(size_t i = 0; i < events.size(); ++ i){
            int fd = events[i].data.fd;
            //情况一：如果监听事件里的fd是serv，说明新客户端接入
            if(fd == serv->getFd()){
                /*
                此处用智能指针会在离开作用域就被杀掉，要用一个容器存放
                容器存放要用移动语义move
                */
                std::unique_ptr<Socket> clnt = std::make_unique<Socket>(serv->accept());
                clnt->setNoBlocking();
                epoll->addFd(clnt.get(), EPOLLIN); //客户端socket使用LT模式
                auto res = clients.try_emplace(clnt->getFd(), std::move(clnt));
                //try_emplace 只在key不存在时，才会真正的构造key
                errif(!res.second, "存在重复的client插入");
            }
            else{
                char buf[1024];
                memset(buf, 0, sizeof(buf));
                ssize_t readNum = recv(fd, buf, sizeof(buf), 0);
                if(readNum > 0){
                    std::cout << "收到来自客户端: " << fd << "的信息: " << buf << std::endl;
                    send(fd, buf, readNum, 0);
                }
                else if(readNum == 0){
                    std::cout << "客户端: " << fd << "链接关闭" << std::endl;
                    clients.erase(fd); //erase会触发~Socket()，自动关闭
                }
                else{ // readNum == -1，此时errno开始写入
                    if(errno == EINTR){ 
                        //EINTR = Erro + Interrupt，被信号中断，继续读取
                        std::cout << "[服务器]持续读取" << std::endl;
                        continue;
                    }
                    else if(errno == EAGAIN || errno == EWOULDBLOCK){
                        //如果此时没有数据可读，则不阻塞，直接返回
                        printf("[服务器]完成读取, errno: %d\n", errno);
                    }
                    else{ //其他错误，关闭连接
                        std::cout << "客户端: " << fd << " 读取错误, 关闭连接" << std::endl;
                        clients.erase(fd);
                    }
                }
            }
        }
    }
}