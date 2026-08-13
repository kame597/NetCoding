#include <sys/socket.h>
#include <sys/select.h>    // select, fd_set
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <vector>

void errif(bool condition, const char *errmsg) {
    if (condition) {
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}

int main() {
    // ---------- 1. 创建监听 socket ----------
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    bind(listen_fd,
         reinterpret_cast<sockaddr*>(&serv_addr),
         sizeof(serv_addr));

    listen(listen_fd, SOMAXCONN);

    std::cout << "select server listening on 127.0.0.1:8888" << std::endl;

    // ---------- 2. 维护当前所有活跃的 client fd ----------
    std::vector<int> client_fds;

    while (true) {
        // ======== 核心：每次循环重建 fd_set 并调用 select ========

        fd_set readfds;                    // 声明"可读事件集合"
        FD_ZERO(&readfds);                // 清空集合
        FD_SET(listen_fd, &readfds);      // 把监听 fd 加入集合 —— 有新连接时"可读"

        int max_fd = listen_fd;           // select 需要知道最大 fd 编号

        // 把所有客户端 fd 也加入集合 —— 客户端发数据时"可读"
        for (int fd : client_fds) {
            FD_SET(fd, &readfds);
            if (fd > max_fd) max_fd = fd;
        }

        // select 会阻塞，直到至少有一个 fd 变成"可读"
        // 参数：最大fd+1, 读集合, 写集合(NULL), 异常集合(NULL), 超时(NULL=永久等)
        // select会修改传入的readfds，因此readfds是以地址传入
        // select是系统调用，因此无法使用readfds的地址，内核会将其内容完整拷贝到内核区域内，造成很大的拷贝花销
        int n_ready = select(max_fd + 1, &readfds, nullptr, nullptr, nullptr);
        errif(n_ready == -1, "select error");

        // 情况1：监听 fd 有可读，说明是有新客户端连接到达，可以将其接收
        if (FD_ISSET(listen_fd, &readfds)) {
            sockaddr_in clnt_addr{};
            socklen_t clnt_addr_len = sizeof(clnt_addr);
            int clnt_fd = accept(listen_fd,
                                 reinterpret_cast<sockaddr*>(&clnt_addr),
                                 &clnt_addr_len);
            errif(clnt_fd == -1, "accept error");

            client_fds.push_back(clnt_fd);    // 加入管理列表

            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clnt_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
            std::cout << "New client fd: " << clnt_fd
                      << ", IP: " << ip_str
                      << ", Port: " << ntohs(clnt_addr.sin_port)
                      << std::endl;
        }

        // 情况2：某个 client fd 可读，说明来自客户端的数据到达
        // select的性能瓶颈，需要对所有已注册的fd逐个进行FD_ISSET
        for (size_t i = 0; i < client_fds.size(); ) {
            int fd = client_fds[i];

            if (FD_ISSET(fd, &readfds)) {
                char buf[1024];
                memset(buf, 0, sizeof(buf));
                ssize_t n = recv(fd, buf, sizeof(buf), 0);

                if (n > 0) {
                    std::cout << "message from client fd " << fd << ": " << buf << std::endl;
                    send(fd, buf, n, 0);            // 原样返回（只发收到的字节数）
                } else if (n == 0) {
                    // 客户端主动断开
                    std::cout << "client fd " << fd << " disconnected" << std::endl;
                    close(fd);
                    client_fds.erase(client_fds.begin() + i);
                    continue;                       // 跳过 i++，下一元素自动前移
                } else {
                    // recv 出错
                    std::cout << "client fd " << fd << " error, closing" << std::endl;
                    close(fd);
                    client_fds.erase(client_fds.begin() + i);
                    continue;
                }
            }
            ++i;
        }
    }

    close(listen_fd);
    return 0;
}
