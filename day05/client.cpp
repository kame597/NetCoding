#include "utils.h"
#include "Socket.h"
#include "InetAddress.h"
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
    while(true){
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        scanf("%s", buf);

        ssize_t written = send(clnt->getFd(), buf, strlen(buf), 0);
        if(written == -1){ // write返回值为-1，说明发生错误
            printf("[客户端]连接已经断联, 无法写操作");
            break;
        }

        memset(buf, 0, sizeof(buf));

        ssize_t read = recv(clnt->getFd(), buf, sizeof(buf), 0);
        if(read > 0){
            printf("[客户端]读取来自服务器的信息: %s\n", buf);
        }else if(read == 0){      //read返回0，表示EOF，通常是服务器断开链接
            printf("[客户端]服务器断开连接\n");
            break;
        }else{     //read返回-1，表示发生错误，按照上文方法进行错误处理
            errif(read == -1, "客户端读取错误");
        }
    }
}