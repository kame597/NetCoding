#pragma once
#include <functional>
#include <memory>
#include "Buffer.h"

class EventLoop;
class Socket;
class Channel;
class Connection{
public:
    enum State{
        Invalid = 1,
        HandShaking,
        Connected,
        Closed,
        Failed,
    };

    Connection(EventLoop* _loop, Socket* _sock);
    ~Connection();

    void Read();
    void Write();
    void Close();

    void SetDeleteConnectionCallback(std::function<void(Socket*)>);
    void SetOnConnectCallback(std::function<void(Connection*)>);

    State GetState(){ return state; }
    Socket* GetSocket(){ return sock.get(); }

    const char* ReadBuffer(){ return readBuffer.c_str(); }
    const char* WriteBuffer(){ return writeBuffer.c_str(); }

    void SetWriteBuffer(const char* str){ writeBuffer.setBuf(str); }
    void GetlineWriteBuffer(){ writeBuffer.getline(); }

    void OnConnect(std::function<void()>); 

private:
    EventLoop* loop;
    std::unique_ptr<Socket> sock;           //客户端的socket
    std::unique_ptr<Channel> channel;       //客户端的channel
    State state{State::Invalid};
    Buffer readBuffer;     //这个缓存是输入缓冲区，存储read的内容
    Buffer writeBuffer;    //该缓存是输出缓冲区，存储write的内容

    std::function<void(Socket*)> delete_connection_callback;
    std::function<void(Connection*)> on_connect_callback;

    void ReadNonBlocking();
    void ReadBlocking();
    void WriteNonBlocking();
    void WriteBlocking();
};  