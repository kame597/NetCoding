#pragma once
#include <string>
#include "common.h"

class Buffer{
public:
    DISALLOW_COPY_AND_MOVE(Buffer);
    Buffer() = default;
    ~Buffer() = default;

    const std::string& buf() const {return buf_;}
    const char* c_str() const;

    void set_buf(const char*); //设置buffer的内容
    void set_buf(const std::string&);

    ssize_t Size() const;

    void Append(const char* _str, size_t _size);

    void Clear();

    // 返回可读数据的起始指针
    const char* Peek() const {return buf_.data();}

    // 可读字节数
    size_t ReadableBytes() const {return buf_.size();}

    // 返回找到"\r\n"后的起始指针，返回的指针指向"\r"
    const char* FindCRLF() const;

    // 消费掉前n字节
    void Retrieve(size_t n){buf_.erase(0,n);}

    // 消费掉指定的指针位置
    void RetrieveUntil(const char* end){Retrieve(end - Peek());}
    
private:
    std::string buf_;
};