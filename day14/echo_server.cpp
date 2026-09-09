#include "include/event_loop.h"
#include "include/tcp_server.h"
#include "include/buffer.h"
#include "include/tcp_connection.h"
#include "include/acceptor.h"
#include <memory>
#include <mutex>
#include <iostream>
#include <functional>

int main(){
    std::unique_ptr<TcpServer> server = std::make_unique<TcpServer>("127.0.0.1", 8888);
    
    server->set_message_callback([](TcpConnection* conn){
        std::cout << "[服务器]从客户端" << conn->fd() << "读取信息:" << conn->read_buffer() << std::endl;
        conn->Send(conn->read_buffer());
    });
    server->Start();
    return 0;
}