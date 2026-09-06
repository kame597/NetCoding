#include "Connection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "utils.h"
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>

/*
*   @brief
*   初始化EventLoop与监听sock，并注册channel到epoll.
*   设置connection状态为Connected
*/
Connection::Connection(EventLoop* _loop, Socket* _sock):
    loop(_loop), sock(_sock), channel(nullptr), readBuffer(){
    channel = std::make_unique<Channel>(_loop, sock->getFd());
    channel->enableReading();
    state = State::Connected;
}

Connection::~Connection(){}

/*
*   @brief
*   读取方法，先清空读缓冲区，执行非阻塞读取
*/
void Connection::Read(){
    errif(state != State::Connected, "Connection未连接!");
    readBuffer.clear();
    ReadNonBlocking();
}

/*
*   @brief
*   写入方法，先非阻塞写入，再清空写入缓冲区
*/
void Connection::Write(){
    errif(state != State::Connected, "Connection未连接!");
    WriteNonBlocking();
    writeBuffer.clear();
}

/*
*   @brief
*   调用删除链接的回调函数
*/
void Connection::Close(){
    state = State::Closed;
}


void Connection::SetDeleteConnectionCallback(std::function<void(Socket*)> cb){
    delete_connection_callback = std::move(cb);
}

/*
*   @brief
*   将创建连接时的callback注入到该Connection的Channel的读事件回调上
*   并基于state的状态，将channel再注册或关闭链接
*/
void Connection::SetOnConnectCallback(std::function<void(Connection*)> cb){
    on_connect_callback = std::move(cb);
    //此处用Lambda表达式包装，是为了实现function<void(Connection*)>到function<void()>转化
    channel->setReadCallback([this](){ 
        on_connect_callback(this); 
        if(state == State::Connected)
            channel->enableReading();
        else if(state == State::Closed)
            delete_connection_callback(sock.get());
    });
}

/*
*   @brief
*   非阻塞读取，该函数只进行读取，不进行业务设计
*/
void Connection::ReadNonBlocking(){
    int sockFd = sock->getFd();
    char buf[1024];
    while(true){
        memset(buf, 0, sizeof(buf));
        ssize_t readBytes = read(sockFd, buf, sizeof(buf));

        if(readBytes > 0){  //如果读到数据，则放到缓冲区中
            readBuffer.append(buf, readBytes);
        }
        else if(readBytes == -1 && errno == EINTR){     //正常程序中断
            continue;
        }
        else if(readBytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){  //程序读取完成，或者没有数据可读了
            break;
        }
        else if(readBytes == 0){    //客户端断开链接
            std::cout << "客户端:" << sockFd <<"断开链接" << std::endl;
            state = State::Closed;
            break;
        }
        else{
            std::cout << "客户端:" << sockFd << "发生其他错误" << std::endl;
            state = State::Closed;
            break;
        }
    }
}

/*
*   @brief
*   阻塞式读取，暂时用不到
*/
void Connection::ReadBlocking(){

}

/*
*   @brief
*   非阻塞写入，只负责写入，不进行业务设计
*   dataSize表示总共要写多少数据，dataLeft表示还剩下多少字节没写完   
*
*/
void Connection::WriteNonBlocking(){
    int sockFd = sock->getFd();
    char buf[writeBuffer.size()];
    memcpy(buf, writeBuffer.c_str(), writeBuffer.size());

    int dataSize = writeBuffer.size();
    int dataLeft = dataSize;

    while(dataLeft > 0){
        ssize_t writeBytes = write(sockFd, buf + dataSize - dataLeft, dataLeft);
        if(writeBytes == -1 && errno == EINTR){
            continue;
        }
        else if(writeBytes == -1 && (errno == EAGAIN)){
            break;
        }
        else if(writeBytes == -1){
            std::cout << "客户端:" << sockFd << "发生其他错误" << std::endl;
            state = State::Closed;
            break;
        }
        dataLeft -= writeBytes;
    }
}

/*
*   @brief
*   阻塞式写入，暂时用不到
*/
void Connection::WriteBlocking(){

}
