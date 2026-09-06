#include "src/utils.h"
#include "src/Socket.h"
#include "src/InetAddress.h"
#include "src/Buffer.h"
#include <src/ThreadPool.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <memory>
#include <iostream>
#include <cstdint>
#include <cstdlib>


void oneClient(int msgs, int wait){
    std::unique_ptr<Socket> clnt = std::make_unique<Socket>();
    std::unique_ptr<InetAddress> clnt_ia = std::make_unique<InetAddress>("127.0.0.1", 8888);
    clnt->connect(clnt_ia.get());

    Buffer writeBuffer;
    Buffer readBuffer;

    sleep(wait);    //sleep是什么意思来着
    int count = 0;

    while(count < msgs){
        writeBuffer.setBuf("I'm Client!");
        ssize_t writeBytes = write(clnt->getFd(), writeBuffer.c_str(), writeBuffer.size());

        if(writeBytes == -1){ // write返回值为-1，说明发生错误
            printf("[客户端]连接已经断联, 无法写操作");
            break;
        }

        int alreadyRead = 0;
        char buf[1024];

        while(true){
            memset(buf, 0, sizeof(buf));
            ssize_t readBytes = read(clnt->getFd(), buf, sizeof(buf));
            if(readBytes > 0){
                readBuffer.append(buf, readBytes);
                alreadyRead += readBytes;
            } else if(readBytes == 0){         //EOF
                std::cout << "[客户端]服务器断开链接" << std::endl;
                exit(EXIT_SUCCESS);
            }
            /*
            *   当already_read等于sendBuffer的大小时，
            *   说明已经完整收到了来自服务器的信息（因为echo返回相同字符串），因此打印信息
            */
            if(alreadyRead >= writeBuffer.size()){  
                std::cout << "[客户端]收到来自服务器的信息: " << readBuffer.c_str() << std::endl;
                break;
            } 
        }
        readBuffer.clear();
        ++ count;
    }
}

int main(int argc, char *argv[]) {
    int threads = 100;
    int msgs = 100;
    int wait = 0;
    int o;
    const char *optstring = "t:m:w:";
    while ((o = getopt(argc, argv, optstring)) != -1) {
        switch (o) {
            case 't':
                threads = std::stoi(optarg);
                break;
            case 'm':
                msgs = std::stoi(optarg);
                break;
            case 'w':
                wait = std::stoi(optarg);
                break;
            case '?':
                printf("error optopt: %c\n", optopt);
                printf("error opterr: %d\n", opterr);
                break;
        }
    }

    ThreadPool *poll = new ThreadPool(threads);
    std::function<void()> func = std::bind(oneClient, msgs, wait);
    for(int i = 0; i < threads; ++i){
        poll->add(func);
    }
    delete poll;
    return 0;
}