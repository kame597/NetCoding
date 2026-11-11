#pragma once
#include <string>
#include <vector>
#include <assert.h>
#include "common.h"

class Buffer{
public:
    static const size_t kCheapPrepend = 8;      // prependable部分字节
    static const size_t kInitialSize = 1024;    // 初始buffer的大小 = kCheapPrepend + kInitialSize

    DISALLOW_COPY_AND_MOVE(Buffer);
    Buffer(size_t initalSize = kInitialSize):
        buffer_(kCheapPrepend + initalSize),
        read_index_(kCheapPrepend),
        write_index_(kCheapPrepend)
    {}
    ~Buffer() = default;

    // 可读字节数
    size_t ReadableBytes() const {return write_index_ - read_index_;}
    // 可写字节数
    size_t WritableBytes() const {return buffer_.size() - write_index_;}
    // 前缀预置空间字节数
    size_t PrependableBytes() const {return read_index_;}

    char* BeginWrite() {return begin() + write_index_;}
    const char* BeginWrite() const {return begin() + write_index_;}

    char* begin() { return buffer_.data(); }
    const char* begin() const {return buffer_.data(); }

    // 返回可读区域首指针
    const char* Peek() const {return begin() + read_index_;}

    void Prepend(const char* data, size_t len);

    void Append(const char* data, size_t len);

    void EnsureWritableBytes(size_t len);

    void ExpandSpace(size_t len);

    ssize_t ReadFd(int fd, int* savedError);
    ssize_t WriteFd(int fd, int* savedError);

    // 消费（读取）指定长度数据
    void Retrieve(size_t len);

    // 消费所有可读数据，即重置读索引和写索引
    void RetrieveAll();

    std::string RetrieveAsString(size_t len);

    void RetrieveUntil(const char* end);

    // 返回找到"\r\n"后的起始指针，返回的指针指向"\r"
    const char* FindCRLF() const;
    
private:
    std::vector<char> buffer_;
    size_t read_index_;
    size_t write_index_;
};