#include "Socket.h"
#include "InetAddress.h"
#include "utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>


Socket::Socket(){
    fd = socket(PF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socket创建失败");
}

Socket::~Socket(){
    if(fd != -1){
        close(fd);
        fd = -1;
    }
}

void Socket::setNoBlocking(){
    errif((fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) < 0), "设置非阻塞失败");
}

void Socket::bind(InetAddress* ia){
    errif((::bind(fd, reinterpret_cast<sockaddr*>(&ia->getAddr()), ia->getLen()) == -1), "地址绑定错误");
}

void Socket::listen(int size){
    errif((::listen(fd, size)) == -1, "socket监听失败");
}

void Socket::connect(InetAddress* ia){
    errif((::connect(fd, reinterpret_cast<sockaddr*>(&ia->getAddr()), ia->getLen())) == -1, "客户端连接失败");
}

int Socket::accept(){
    sockaddr_in clnt_addr{};
    socklen_t len = sizeof(clnt_addr);
    int clnt_fd = ::accept(fd, reinterpret_cast<sockaddr*>(&clnt_addr), &len);
    return clnt_fd;
}



