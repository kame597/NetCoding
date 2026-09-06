#pragma once
#include <arpa/inet.h>
#include <cstdint>
class InetAddress{
private:
    struct sockaddr_in addr;
    socklen_t addr_len;
public:
    InetAddress() = default;
    InetAddress(const char* ip, uint16_t port);
    ~InetAddress() = default;
    struct sockaddr_in& getAddr(){return addr;}
    const struct sockaddr_in& getAddr() const{return addr;}
    socklen_t getLen(){return addr_len;}
    socklen_t getLen() const{return addr_len;}
};