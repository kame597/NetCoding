#pragma once
class InetAddress;
class Socket{
private:
    int fd = -1;
public:
    Socket();
    explicit Socket(int fd) : fd(fd){}
    ~Socket();
    
    int getFd(){return fd;}
    void setNoBlocking();
    void bind(InetAddress*);
    void listen(int size);
    void connect(InetAddress*);
    int accept();    
};