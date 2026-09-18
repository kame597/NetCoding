#pragma once
#include "common.h"
#include <functional>
#include <memory>
#include "buffer.h"

/*
*   enable_shared_from_this是C++11引入的模版类，定义于memory中
*   其功能为：让一个已经被 std::shared_ptr 管理的对象，
*   能够安全地在成员函数内部获得一个指向自身的 std::shared_ptr
*   
*   在TcpConnection在被shared_ptr管理后，调用shared_from_this，就能得到一个自身的shared_ptr
*   是CRTP模版，让TcpConnection能在成员函数中调用shared_from_this拿到自身
*/
class TcpConnection : public std::enable_shared_from_this<TcpConnection>{
public:
    enum ConnectionState{
        Invalid = 1,
        Connected,
        DisConnected
    };

    DISALLOW_COPY_AND_MOVE(TcpConnection);

    TcpConnection(EventLoop* loop, int connfd, int connid);
    ~TcpConnection();

    void ConnectionEstablish();

    void ConnectionDestruct();

    
    void set_close_callback(std::function<void (std::shared_ptr<TcpConnection>)>const& cb){ on_close_ = std::move(cb); }
    void set_message_callback(std::function<void (std::shared_ptr<TcpConnection>)>const& cb){ on_message_ = std::move(cb); }
    void set_connect_callback(std::function<void (std::shared_ptr<TcpConnection>)>const& cb){ on_connect_ = std::move(cb); }

    void set_write_buf(const char* buf){ write_buffer_.set_buf(buf); }
    const char* read_buffer(){ return read_buffer_.c_str(); }
    const char* write_buffer(){ return write_buffer_.c_str(); }

    void Read();    //读操作
    void Write();   //写操作
    void Send(const std::string& msg);
    void Send(const char* msg, int len);
    void Send(const char* msg);

   void HandleMessage();    //当接受到信息时，触发回调
   void HandleClose();      

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
    Buffer read_buffer_;                          //这个缓存是输入缓冲区，存储read的内容
    Buffer write_buffer_;                         //该缓存是输出缓冲区，存储write的内容


    //三个业务回调，分别用于关闭时触发，收信息，建立连接，由TcpServer注入
    std::function<void(const std::shared_ptr<TcpConnection> &)> on_close_;
    std::function<void(const std::shared_ptr<TcpConnection> &)> on_message_;
    std::function<void(const std::shared_ptr<TcpConnection> &)> on_connect_;

    void ReadNonBlocking();
    void WriteNonBlocking();
};  