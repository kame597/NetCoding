#include "tcp_connection.h"
#include "event_loop.h"
#include "channel.h"
#include "utils.h"

#include <sys/socket.h>

#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>


TcpConnection::TcpConnection(EventLoop* loop, int connfd, uint64_t connid):
    connfd_(connfd), connid_(connid), conn_state_(ConnectionState::Connected), loop_(loop){

    if(loop != nullptr){
        conn_channel_ = std::make_unique<Channel>(connfd_, loop);
        conn_channel_->set_read_callback(std::bind(&TcpConnection::HandleMessage, this));
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
    conn_channel_->Tie(shared_from_this());     //conn_channel绑定了TcpConnection的指针，保存了一个引用计数，用于延迟删除
    conn_channel_->EnableRead();
    if(on_connection_)
        on_connection_(shared_from_this());
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
    if(conn_state_ != ConnectionState::DisConnected)
        ResetIdleTimer(); // 如果该链接有读取，且读取成功，连接还在，则重置定时器
    if(on_message_)
        on_message_(shared_from_this(), &read_buffer_);
}

void TcpConnection::HandleClose(){
    if(conn_state_ != ConnectionState::DisConnected){
        conn_state_ = ConnectionState::DisConnected;
        /*
        *   先关闭读监听：
        *   避免关闭后到DeleteChannel摘除fd的窗口期内，epoll仍触发读事件导致空转
        */
        conn_channel_->DisableRead();   
        CancelIdleTimer();  //取消定时器，删掉RunAfter存的TcpConnection引用
        if(on_close_)
            on_close_(shared_from_this());
    }
}

void TcpConnection::ForceClose(){
    if(conn_state_ != ConnectionState::DisConnected){   //幂等保护，防止多次调用
        HandleClose();
        if(shutdown(connfd_, SHUT_WR) == -1) // shutdown可以明确给对端发FIN，并置为半关闭状态
            std::cout << "[ForceClose]shutdown失败, fd: " << connfd_ << std::endl;
    }
}

/*
*   @brief
*   启用空闲定时器，开启定时关闭任务
*/
void TcpConnection::EnableIdleTimeout(int seconds){
    if(seconds <= 0) return;    // 保护，防止传入seconds = 0，导致所有立刻触发ForceClose强制断开链接
    if(idle_timer_fd_ != -1) return;
    conn_timeout_ = seconds;
    idle_timer_fd_ = loop_->RunAfter(conn_timeout_, std::bind(&TcpConnection::ForceClose, shared_from_this()));
}

/*
 *   @brief
 *   重置空闲定时器
 */
void TcpConnection::ResetIdleTimer(){
    // 先删除原有的定时器，重新发布定时任务
    if(conn_state_ != ConnectionState::Connected) return;
    if(conn_timeout_ <= 0) return;
    loop_->CancelTimer(idle_timer_fd_);
    idle_timer_fd_ = loop_->RunAfter(conn_timeout_, std::bind(&TcpConnection::ForceClose, shared_from_this()));
    
}

void TcpConnection::CancelIdleTimer(){
    if(idle_timer_fd_ != -1){
        loop_->CancelTimer(idle_timer_fd_);
        idle_timer_fd_ = -1;
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
    // read_buffer_.Clear();   为适应Http的半包行为，改装成累积缓冲区，不再清空
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