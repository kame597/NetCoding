#include "buffer.h"
#include <iostream>
#include <algorithm>    // search, copy

#include <sys/uio.h>    // iovec, 是 POSIX 标准中定义的一个结构体类型，用于描述一块内存区域，是 readv/writev 这类分散/聚集 I/O 函数的基础。
#include <unistd.h>

/*
struct iovec {
    void  *iov_base;    // 指向内存区域的起始地址
    size_t iov_len;     // 该内存区域的字节长度
};
readv/writev 等函数接收一个 iovec 结构体数组，以及数组的元素个数，从而可以对多块不连续的内存进行一次性读取或写入。
*/

void Buffer::Prepend(const char *data, size_t len){
    assert(len <= PrependableBytes());
    read_index_ -= len;
    std::copy(data, data + len, begin() + read_index_);    // copy：前两个参数为一个输入区间范围，第三个参数为输出起点，是泛型算法，此处等价于memcpy(begin() + read_index_, d, len);
}

void Buffer::EnsureWritableBytes(size_t len)
{
    if(WritableBytes() < len){
        ExpandSpace(len);
    }
    assert(WritableBytes() >= len);
}

void Buffer::Append(const char *data, size_t len)
{
    EnsureWritableBytes(len);
    std::copy(data, data + len, BeginWrite());   
    write_index_ += len;
}

void Buffer::ExpandSpace(size_t len){
    // 扩容分为真扩容和伪扩容，真扩容发生在前方+后方空闲空间拼起来都不够的情形，需要实际的分配内存
    // 伪扩容就是前后方空闲空间足够，只需要移动index即可实现分配

    if(WritableBytes() + PrependableBytes() < len + kCheapPrepend){ // 剩余空闲不足，分配空间
        buffer_.resize(write_index_ + len);     // 将缓冲区大小设为writeIndex_ + len，保证从writeIndex_后，恰好有len字节可写
    }
    else{   // 如果剩余空间足够，则拼接首尾空闲空间(首部空闲空间，主要是read_index在retrieve操作后右移产生的)
        size_t readable = ReadableBytes();
        std::copy(begin() + read_index_, begin() + write_index_, begin() + kCheapPrepend);
        read_index_ = kCheapPrepend;    // 重置read_index_
        write_index_ = kCheapPrepend + readable;
    }
}

ssize_t Buffer::ReadFd(int fd, int *savedError)
{
    char extraBuf[65535]{};
    struct iovec vec[2];
    const size_t writable = WritableBytes();

    vec[0].iov_base = BeginWrite();
    vec[0].iov_len = writable;

    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf);

    // 语义：如果可写大小大于65535，则足够够用，不需要让extraBuf参与，否则引入extraBuf
    const int iovcnt = (writable < sizeof(extraBuf)) ? 2 : 1;   // 这一句？

    /*
    *   readv:POSIX的分散读（scatter read）系统调用, 从同一个文件描述符里读数据，按顺序依次填满多个不连续的内存块。
    *   fd为要读的文件描述符，vec是iovec，iovcnt为读取的数组元素个数   
    */
    ssize_t nread = ::readv(fd, vec, iovcnt);   // 返回值代表实际读取字节数，0代表对端关闭，-1代表出错
    if(nread < 0){
        *savedError = errno;
    }
    else if(nread <= writable){
        write_index_ += nread;
    }
    else{   // 读取字节数大于可写区域，则将extraBuf存的内容移进缓冲区，并进行Append扩容
        write_index_ = buffer_.size();
        Append(extraBuf, nread - writable);
    }
    return nread;
}

ssize_t Buffer::WriteFd(int fd, int *savedError)
{
    if(ReadableBytes() == 0) return 0;      // 防御，如果是空 buffer 则不调 write
    ssize_t nwrite = ::write(fd, Peek(), ReadableBytes());
    if(nwrite < 0){
        *savedError = errno;
    }
    else{   // 如果有写入，则消费缓冲区
        Retrieve(nwrite);
    }
    return nwrite;
}

/*
*   @brief
*   消费指定长度数据
*/
void Buffer::Retrieve(size_t len){
    if(len < ReadableBytes()){   // 如果可读字节数够用，则只移动指针
        read_index_ += len;
    }
    else{   // 如果指定长度大于等于可读字节数，则全部取出，本质也是只移动指针
        RetrieveAll();  
    }
}

void Buffer::RetrieveAll(){
    read_index_ = kCheapPrepend;
    write_index_ = kCheapPrepend;
}

std::string Buffer::RetrieveAsString(size_t len){
    assert(len <= ReadableBytes());
    std::string result(Peek(), len);    //使用char*, size_t的初始化方式
    Retrieve(len);
    return result;
}

void Buffer::RetrieveUntil(const char *end){
    assert(end >= Peek() && end <= Peek() + ReadableBytes());
    Retrieve(static_cast<size_t>(end - Peek()));
}


const char *Buffer::FindCRLF() const
{
    // search函数用于查找子串
    const char* crlf = std::search(Peek(), BeginWrite(), "\r\n", "\r\n"+2);
    return crlf == BeginWrite() ? nullptr : crlf;
}
