#include "include/thread_pool.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <iostream>
#include <string>
#include <functional>

// 单个客户端：连接服务器，发送 msgs 条消息，并接收 echo 回显
void oneClient(int msgs, int wait){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        std::cout << "[客户端]socket创建失败" << std::endl;
        return;
    }

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if(connect(sockfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1){
        std::cout << "[客户端]连接服务器失败" << std::endl;
        close(sockfd);
        return;
    }

    sleep(wait);   // 让客户端错峰连接，避免同时涌入
    int count = 0;
    const std::string msg = "I'm Client!";

    while(count < msgs){
        ssize_t writeBytes = write(sockfd, msg.c_str(), msg.size());
        if(writeBytes == -1){
            std::cout << "[客户端]连接已断联，无法写操作" << std::endl;
            break;
        }

        std::string recvStr;
        char buf[1024];
        int alreadyRead = 0;

        while(true){
            memset(buf, 0, sizeof(buf));
            ssize_t readBytes = read(sockfd, buf, sizeof(buf));
            if(readBytes > 0){
                recvStr.append(buf, readBytes);
                alreadyRead += readBytes;
            } else if(readBytes == 0){          // EOF，服务器关闭连接
                std::cout << "[客户端]服务器断开连接" << std::endl;
                close(sockfd);
                return;
            } else if(readBytes == -1 && errno == EINTR){
                continue;
            }
            // 读满和发送等长的字节，说明完整收到 echo 回显
            if(alreadyRead >= static_cast<int>(msg.size())){
                std::cout << "[客户端]收到来自服务器的信息: " << recvStr << std::endl;
                break;
            }
        }
        ++count;
    }
    close(sockfd);
}

int main(int argc, char* argv[]){
    int threads_ = 1000;   // 客户端线程数
    int msgs = 100;       // 每个客户端发送的消息数
    int wait = 0;         // 每个客户端连接后等待的秒数
    int o;
    const char* optstring = "t:m:w:";
    while((o = getopt(argc, argv, optstring)) != -1){
        switch(o){
            case 't':
                threads_ = std::stoi(optarg);
                break;
            case 'm':
                msgs = std::stoi(optarg);
                break;
            case 'w':
                wait = std::stoi(optarg);
                break;
            case '?':
                std::cout << "用法: ./client [-t 线程数] [-m 消息数] [-w 等待秒]" << std::endl;
                break;
        }
    }

    ThreadPool* pool = new ThreadPool(threads_);
    std::function<void()> func = std::bind(oneClient, msgs, wait);
    for(int i = 0; i < threads_; ++i){
        pool->Add(func);
    }
    delete pool;   // 析构会 join 所有线程，等待全部压测任务完成
    return 0;
}
