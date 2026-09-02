#include "src/utils.h"
#include "src/Socket.h"
#include "src/InetAddress.h"
#include "src/Buffer.h"
#include <sys/socket.h>
#include <cstring>
#include <memory>
#include <iostream>
#include <cstdint>
#include <string>

int main(){
    std::unique_ptr<Socket> clnt = std::make_unique<Socket>();
    std::unique_ptr<InetAddress> clnt_ia = std::make_unique<InetAddress>("127.0.0.1", 8888);
    clnt->connect(clnt_ia.get());

    Buffer sendBuffer;
    Buffer readBuffer;

    while(true){
        sendBuffer.getline();
        ssize_t written = send(clnt->getFd(), sendBuffer.c_str(), sendBuffer.size(), 0);
        if(written == -1){ // write返回值为-1，说明发生错误
            printf("[客户端]连接已经断联, 无法写操作");
            break;
        }

        int already_read = 0;
        char buf[1024];     //该buf的大小无关紧要

        while(true){
            memset(buf, 0, sizeof(buf));
            ssize_t read_bytes = recv(clnt->getFd(), buf, sizeof(buf), 0);
            if(read_bytes > 0){
                readBuffer.append(buf, read_bytes);
                already_read += read_bytes;
            } else if(read_bytes == 0){         //EOF
                std::cout << "[客户端]服务器断开链接" << std::endl;
                exit(EXIT_SUCCESS);
            }
            /*
            当already_read等于sendBuffer的大小时，
            说明已经完整收到了来自服务器的信息（因为echo返回相同字符串），因此打印信息
            */
            if(already_read >= sendBuffer.size()){  
                std::cout << "[客户端]收到来自服务器的信息: " << readBuffer.c_str() << std::endl;
                break;
            } 
        }
        readBuffer.clear();
    }
    return 0;
}