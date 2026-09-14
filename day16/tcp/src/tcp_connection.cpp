#include "tcp_connection.h"
#include "event_loop.h"
#include "channel.h"
#include "utils.h"
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>

TcpConnection::TcpConnection(EventLoop* loop, int connfd, int connid):
    connfd_(connfd), connid_(connid), conn_state_(ConnectionState::Connected), loop_(loop){

    if(loop != nullptr){
        conn_channel_ = std::make_unique<Channel>(connfd_, loop);
        conn_channel_->set_read_callback(std::bind(&TcpConnection::HandleMessage, this));
        conn_channel_->EnableRead();
    }
}

TcpConnection::~TcpConnection(){
}

/*
*   @brief  
*   建立TcpConnection时，首先将其绑定在Channel的tie_上;
*   这部分专门分离出来，因为shared_from_this是无法在构造函数中调用的
*/
void TcpConnection::ConnectionEstablish(){
    conn_state_ = ConnectionState::Connected;
    conn_channel_->Tie(shared_from_this());     //conn_channel绑定了TcpConnection的指针，保存了一个引用计数，用于延迟删除
    conn_channel_->EnableRead();
    if(on_connect_)
        on_connect_(shared_from_this());
}

/*
*   @brief  
*   又一层封装，将对应fd从epoll表中移除
*   保证TcpConnection是在HandleEvent之后才析构
*/
void TcpConnection::ConnectionDestruct(){
    loop_->DeleteChannel(conn_channel_.get());
}

void TcpConnection::HandleMessage(){
    Read();
    if(on_message_)
        on_message_(shared_from_this());
}

void TcpConnection::HandleClose(){
    if(conn_state_ != ConnectionState::DisConnected){
        conn_state_ = ConnectionState::DisConnected;
        /*
        *   先关闭读监听：
        *   避免关闭后到DeleteChannel摘除fd的窗口期内，epoll仍触发读事件导致空转
        */
        conn_channel_->DisableRead();   
        if(on_close_)
            on_close_(shared_from_this());
    }
}


void TcpConnection::Send(const std::string& msg){
    set_write_buf(msg.c_str());
    Write();
}

void TcpConnection::Send(const char* msg){
    set_write_buf(msg);
    Write();
}

void TcpConnection::Send(const char* msg, int len){
    write_buffer_.Append(msg, len);
    Write();
}

void TcpConnection::Read(){
    if(conn_state_ != ConnectionState::Connected) return;
    read_buffer_.Clear();
    ReadNonBlocking();
}

void TcpConnection::Write(){
    if(conn_state_ != ConnectionState::Connected) return;
    WriteNonBlocking();
    write_buffer_.Clear();
}


void TcpConnection::ReadNonBlocking(){
    char buf[1024];
    while(true){
        memset(buf, 0, sizeof(buf));
        ssize_t read_bytes = read(connfd_, buf, sizeof(buf));

        if(read_bytes > 0){  //如果读到数据，则放到缓冲区中
            read_buffer_.Append(buf, read_bytes);
        }
        else if(read_bytes == -1 && errno == EINTR){     //正常程序中断
            continue;
        }
        else if(read_bytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){  //程序读取完成，或者没有数据可读了
            break;
        }
        else if(read_bytes == 0){    //客户端断开链接
            std::cout << "客户端:" << connfd_ <<"断开链接" << std::endl;
            HandleClose();
            break;
        }
        else{
            std::cout << "客户端:" << connfd_ << "发生其他错误" << std::endl;
            HandleClose();
            break;
        }
    }
}

/*
*   @brief
*   非阻塞写入，只负责写入，不进行业务设计
*   dataSize表示总共要写多少数据，dataLeft表示还剩下多少字节没写完   
*
*/
void TcpConnection::WriteNonBlocking(){
    char buf[write_buffer_.Size()];
    memcpy(buf, write_buffer_.c_str(), write_buffer_.Size());

    int data_size = write_buffer_.Size();
    int data_left = data_size;

    while(data_left > 0){
        ssize_t write_bytes = write(connfd_, buf + data_size - data_left, data_left);
        if(write_bytes == -1 && errno == EINTR){
            continue;
        }
        else if(write_bytes == -1 && (errno == EAGAIN)){
            break;
        }
        else if(write_bytes == -1){
            std::cout << "客户端:" << connfd_ << "发生其他错误" << std::endl;
            HandleClose();
            break;
        }
        data_left -= write_bytes;
    }
}