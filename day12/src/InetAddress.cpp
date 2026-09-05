#include "InetAddress.h"

InetAddress::InetAddress(const char* ip, uint16_t port){
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(port);
    addr_len = sizeof(addr);
}