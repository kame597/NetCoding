#pragma once
#include "common.h"
#include <functional>
#include <memory>
#include "buffer.h"

class TcpConnection{
public:
    enum ConnectionState{
        Invalid = 1,
        Connected,
        DisConnected
    };

    DISALLOW_COPY_AND_MOVE(TcpConnection);

    TcpConnection(EventLoop* loop, int connfd, int connid);
    ~TcpConnection();

    // 关闭时的回调函数
    void set_close_callback(std::function<void(int)>const& cb){ on_close_ = std::move(cb); }
    // 接受信息的回调函数
    void set_message_callback(std::function<void(TcpConnection*)>const& cb){ on_message_ = std::move(cb); }

    void set_write_buf(const char* buf){ write_buffer_.set_buf(buf); }
    const char* read_buffer(){ return read_buffer_.c_str(); }
    const char* write_buffer(){ return write_buffer_.c_str(); }

    void Read();    //读操作
    void Write();   //写操作
    void Send(const std::string& msg);
    void Send(const char* msg, int len);
    void Send(const char* msg);

   void HandleMessage();    //当接受到信息时，触发回调
   void HandleClose();      //当TcpConnection时发起关闭请求，进行回调，释放相应的socket

    ConnectionState conn_state()const { return conn_state_; }
    EventLoop* loop()const {return loop_;}
    int fd()const { return connfd_; }
    int id()const { return connid_; }

private:
    int connfd_;
    int connid_;
    ConnectionState conn_state_;

    EventLoop* loop_;

    std::unique_ptr<Channel> conn_channel_;       //客户端的channel
    Buffer read_buffer_;                            //这个缓存是输入缓冲区，存储read的内容
    Buffer write_buffer_;                           //该缓存是输出缓冲区，存储write的内容

    std::function<void(int)> on_close_;
    std::function<void(TcpConnection*)> on_message_;

    void ReadNonBlocking();
    void WriteNonBlocking();
};  