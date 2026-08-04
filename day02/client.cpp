#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

void errif(bool condition, const char *errmsg){
    if(condition){
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}

int main(){
    //客户端与服务器基本相同，
    //创建一个socket文件描述符，与一个IP地址和端口绑定
    //但最后并不是监听这个端口，而是使用`connect`函数尝试连接这个服务器。

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //connect部分，“拨号”
    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); //将 "127.0.0.1" 这个点分十进制字符串转为网络字节序的二进制，存入 sin_addr
    connect(sockfd,
            reinterpret_cast<sockaddr*>(&serv_addr),
            sizeof(serv_addr));

    while(true){
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        scanf("%s", buf);
        ssize_t write_bytes = write(sockfd, buf, sizeof(buf)); //发送缓冲区中的数据到服务器socket，返回已发送数据大小

        if(write_bytes == -1){ // write返回值为-1，说明发生错误
            printf("socket already disconnected, can't write!!!");
            break;
        }

        memset(buf, 0, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if(read_bytes > 0){
        printf("message from server: %s\n", buf);
        }else if(read_bytes == 0){      //read返回0，表示EOF，通常是服务器断开链接
        printf("server socket disconnected!\n");
        break;
        }else if(read_bytes == -1){     //read返回-1，表示发生错误，按照上文方法进行错误处理
        close(sockfd);
        errif(true, "socket read error");
        }
    }
    close(sockfd);
    return 0;
}