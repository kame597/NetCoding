#include "InetAddress.h"
#include "Socket.h"
#include "Epoll.h"
#include "utils.h"
#include "Channel.h"
#include <sys/socket.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <utility> //存放了标准库move函数，用于获得绑定到左值上的右值引用
#include <cstring>
#include <iostream>

/*
结构体Client
用于绑定客户端连接的sock和channel，防止服务器新建连接后离开作用域后，指针错误删除
*/
struct Client{
    std::unique_ptr<Socket> sock;
    std::unique_ptr<Channel> ch;
};

int main(){
    std::unique_ptr<Socket> serv = std::make_unique<Socket>();
    std::unique_ptr<InetAddress> serv_addr = std::make_unique<InetAddress>("127.0.0.1", 8888);
    serv->bind(serv_addr.get());
    serv->listen(SOMAXCONN);
    serv->setNoBlocking(); //设置为非阻塞模式

    std::unique_ptr<Epoll> epoll = std::make_unique<Epoll>(0);
    /*
    这个版本下不需要用Epoll类执行epoll红黑树注册，而是通过channel管理注册
    // epoll->addFd(serv.get(), EPOLLIN);
    */
    
    // clients 用于所有存储客户端的连接
    std::unordered_map<int, Client> clients;

    // servChannel 是服务器频道，用于封装epoll与对应fd
    std::unique_ptr<Channel> servChannel = std::make_unique<Channel>(epoll.get(), serv->getFd()); 
    servChannel->enableReading();

    while(true){
        std::vector<Channel*> activeChannel = epoll->poll();
        for(size_t i = 0; i < activeChannel.size(); ++ i){
            int fd = activeChannel[i]->getFd();
            //情况一：如果监听事件里的fd是serv，说明新客户端接入
            if(fd == serv->getFd()){
                /*
                此处用智能指针会在离开作用域就被杀掉，要用一个容器存放
                容器存放要用移动语义move
                */
                std::unique_ptr<Socket> clntSock = std::make_unique<Socket>(serv->accept());
                int clntFd = clntSock->getFd();
                clntSock->setNoBlocking();
                
                std::unique_ptr<Channel> clntChannel = std::make_unique<Channel>(epoll.get(), clntSock->getFd());
                clntChannel->enableReading();

                Client c;
                c.sock = std::move(clntSock);
                c.ch = std::move(clntChannel);

                //此处为什么channel初始化用epoll.get()呢？这个epoll不是服务器注册时用了吗？
                auto res = clients.emplace(clntFd, std::move(c));
                errif(!res.second, "存在重复的client插入");
            }
            //情况二：如果该事件是可读事件
            else if(activeChannel[i]->getRevent() & EPOLLIN){
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
            else{
                //待实现
                std::cout << "其他类型事件，待实现..." << std::endl;
            }
        }
    }
    return 0;
}