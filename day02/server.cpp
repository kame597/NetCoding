#include <sys/socket.h>   // socket API (socket/bind/listen/accept)
#include <arpa/inet.h>    // inet_pton, htons/htonl/ntohs/ntohl
#include <cstring>        // std::memset
#include <iostream>       // std::cerr
#include <unistd.h>


 void errif(bool condition, const char *errmsg){
    if(condition){ 
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}

int main()
{
    
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    bind(sockfd,
         reinterpret_cast<sockaddr*>(&serv_addr),
         sizeof(serv_addr));

    // 开始监听，SOMAXCONN = Socket MAX CONNections，系统定义的监听队列最大长度（通常 128 或 4096）
    listen(sockfd, SOMAXCONN);

    sockaddr_in clnt_addr{};                                   
    socklen_t clnt_addr_len = sizeof(clnt_addr); 
    int clnt_sockfd = accept(sockfd,
                             reinterpret_cast<sockaddr*>(&clnt_addr), 
                             &clnt_addr_len);

    //   将网络字节序二进制 IP 转回点分十进制字符串
    char clnt_ip_str[INET_ADDRSTRLEN];  // INET_ADDRSTRLEN = IPv4 地址字符串最大长度（16）
    inet_ntop(AF_INET, &clnt_addr.sin_addr, clnt_ip_str, INET_ADDRSTRLEN);

    std::cout <<  "New client fd: " << clnt_sockfd
              << ", IP: " << clnt_ip_str
              << ", Port: " << ntohs(clnt_addr.sin_port)
              << std::endl;

    while(true){
        //声明并清空缓冲区
        char buf[1024];
        memset(&buf, 0, sizeof(buf));

        //从clnt_sockfd读sizeof(buf)的数据到buf中
        ssize_t read_bytes = read(clnt_sockfd, buf, sizeof(buf));

        if(read_bytes > 0){
            printf("message from client fd %d: %s\n", clnt_sockfd, buf);
            write(clnt_sockfd, buf, sizeof(buf));
        }else if(read_bytes == 0){ //read返回0，说明EOF
            printf("client fd %d disconnected\n", clnt_sockfd);
            close(clnt_sockfd);
            break;
        }else if(read_bytes == -1){ //read返回-1，表示发生错误
            close(clnt_sockfd);
            errif(true, "socket read failure"); //打印错误信息
        }
    }

    return 0;
}