#include <sys/socket.h>   // socket API (socket/bind/listen/accept)
#include <arpa/inet.h>    // inet_pton, htons/htonl/ntohs/ntohl
#include <cstring>        // std::memset
#include <iostream>       // std::cerr

/*
 * 缩写速查表（Socket 编程常用缩写）
 * ──────────────────────────────────────────────
 * fd        = File Descriptor（文件描述符），Linux 万物皆文件，套接字也是文件
 * sock      = Socket（套接字）
 * addr      = Address（地址）
 * in        = Internet（互联网）
 * AF        = Address Family（地址族），决定协议类型
 * INET      = Internet 协议族，AF_INET = IPv4, AF_INET6 = IPv6
 * SOCK      = Socket 类型
 * STREAM    = 流式传输 → TCP（可靠、有序、面向连接）
 * DGRAM     = Datagram（数据报）→ UDP（不可靠、无连接、保边界）
 * sin       = Socket INternet（sockaddr_in 的缩写前缀）
 * sin_family    = 地址族（填 AF_INET）
 * sin_port      = 端口号（网络字节序存储）
 * sin_addr      = IP 地址
 * s_addr        = Socket Address（in_addr 结构体内部的字段名）
 * hton      = Host TO Network（主机字节序(小端序) → 网络字节序（大端序））
 * ntohs      = Network TO Host（网络字节序 → 主机字节序）
 * htons     = Host TO Network Short（16位，通常用于端口号）
 * htonl     = Host TO Network Long（32位，通常用于 IP 地址）
 * SOMAXCONN = Socket MAX CONNections（系统允许的最大监听队列长度）
 * inet_pton = Internet Presentation TO Network（点分十进制字符串 → 网络字节序二进制）
 * inet_ntop = Internet Network TO Presentation（网络字节序二进制 → 点分十进制字符串）
 * inet_ntoa = Internet Network TO ASCII (网络字节序二进制 → 可打印的ASCII码)，已废弃
 */

int main()
{
    
    // socket(协议族, 传输方式, 协议)
    //   - AF_INET (Address Family - Internet) : 使用 IPv4
    //   - SOCK_STREAM                        : 流式套接字，TCP
    //   - 0                                  : 自动选择协议 (IPPROTO_TCP)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // ↑ sockfd = socket file descriptor（套接字文件描述符）


    // 绑定地址与端口
    // sockaddr_in = socket address (internet)，即 IPv4 套接字地址结构体
    struct sockaddr_in serv_addr;   // serv_addr = server address
    std::memset(&serv_addr, 0, sizeof(serv_addr));  // 清零，避免残留垃圾数据

    serv_addr.sin_family = AF_INET;    // 地址族 = IPv4，sin_family = socket internet family

    serv_addr.sin_port = htons(8888);  // 端口 8888，转为网络字节序（大端）

    // ★ 现代做法：用 inet_pton 替代已废弃的 inet_addr
    //   inet_pton = Internet Presentation TO Network
    //   将 "127.0.0.1" 这个点分十进制字符串转为网络字节序的二进制，存入 sin_addr
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    // sin_addr = socket internet address

    bind(sockfd,
         reinterpret_cast<sockaddr*>(&serv_addr),  // C++ 风格类型转换，替代 C 风格强转
         sizeof(serv_addr));
    // sockaddr 是通用套接字地址结构体，sockaddr_in 是 IPv4 专用，需要转换


    // 开始监听，SOMAXCONN = Socket MAX CONNections，系统定义的监听队列最大长度（通常 128 或 4096）
    listen(sockfd, SOMAXCONN);



/*
服务器对客户端CLIENT的接受配置
*/    
    // 花括号自动将结构体清零，无需手动 memset
    sockaddr_in clnt_addr{};                                   
    socklen_t clnt_addr_len = sizeof(clnt_addr); 

    int clnt_sockfd = accept(sockfd,
                             reinterpret_cast<sockaddr*>(&clnt_addr),  //reinterpret：强制转换
                             &clnt_addr_len);

 
    //   将网络字节序二进制 IP 转回点分十进制字符串
    char clnt_ip_str[INET_ADDRSTRLEN];  // INET_ADDRSTRLEN = IPv4 地址字符串最大长度（16）
    inet_ntop(AF_INET, &clnt_addr.sin_addr, clnt_ip_str, sizeof(clnt_ip_str));

    std::cout << "New client fd: " << clnt_sockfd
              << ", IP: " << clnt_ip_str
              << ", Port: " << ntohs(clnt_addr.sin_port)
              << std::endl;

    return 0;
}