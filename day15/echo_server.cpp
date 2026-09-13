#include "include/event_loop.h"
#include "include/tcp_server.h"
#include "include/buffer.h"
#include "include/tcp_connection.h"
#include "include/acceptor.h"

#include <arpa/inet.h>

#include <memory>
#include <mutex>
#include <iostream>
#include <functional>

/*
*   EchoServer
*   是一个短连接echo服务器，收到一句，原样回一句，关闭连接
*/
class EchoServer{
public:
    EchoServer(EventLoop* loop, const char* ip, const int port);
    ~EchoServer();

    void Start(){ server_.Start();}
    void OnConnection(const std::shared_ptr<TcpConnection>& conn);
    void OnMessage(const std::shared_ptr<TcpConnection>& conn);
private:
    TcpServer server_;
};

EchoServer::EchoServer(EventLoop* loop, const char* ip, const int port):server_(loop, ip, port){
    server_.set_connection_callback(std::bind(&EchoServer::OnConnection, this, std::placeholders::_1));
    server_.set_message_callback(std::bind(&EchoServer::OnMessage, this, std::placeholders::_1));
}

EchoServer::~EchoServer(){}

void EchoServer::OnConnection(const std::shared_ptr<TcpConnection>& conn){
    int clnt_fd = conn->fd();
    struct sockaddr_in peer_addr{};
    socklen_t peer_addrlenth = sizeof(peer_addr);

    //getpeername，用于查询连接对端的IP和端口信息，此处是获取客户端的信息，并打印日志
    getpeername(clnt_fd, reinterpret_cast<sockaddr*>(&peer_addr), &peer_addrlenth);

    std::cout << current_thread::tid()
            << " EchoServer::OnNewConnection : new connection "
            << "[fd#" << clnt_fd << "]"
            << " from " << inet_ntoa(peer_addr.sin_addr) << ":" << ntohs(peer_addr.sin_port)
            << std::endl;
}

void EchoServer::OnMessage(const std::shared_ptr<TcpConnection>& conn){
    if(conn->conn_state() == TcpConnection::ConnectionState::Connected){
        std::cout << current_thread::tid() << "Message from clent " << conn->read_buffer() << std::endl;
        conn->Send(conn->read_buffer());
        conn->HandleClose();
    }
}

int main(int argc, char* argv[]){
    int port;
    if(argc <= 1){
        port = 8888;
    }
    else if(argc == 2){
        port = std::stoi(argv[1]);
    }
    else{
        printf("请输入正确端口号");
        exit(0);
    }
    std::unique_ptr<EventLoop> loop = std::make_unique<EventLoop>();
    std::unique_ptr<EchoServer> server = std::make_unique<EchoServer>(loop.get(), "127.0.0.1", port);
    server->Start();
    return 0;
}

// int main(){
//     std::unique_ptr<EventLoop> loop = std::make_unique<EventLoop>();
//     std::unique_ptr<TcpServer> server = std::make_unique<TcpServer>(loop.get(), "127.0.0.1", 8888);
    
//     server->set_message_callback([](std::shared_ptr<TcpConnection> conn){
//         std::cout << "[服务器]从客户端" << conn->fd() << "读取信息:" << conn->read_buffer() << std::endl;
//         conn->Send(conn->read_buffer());
//     });
//     server->Start();
//     return 0;
//}