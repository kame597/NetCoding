#include <sys/socket.h>
#include <arpa/inet.h>

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

    
    return 0;
}