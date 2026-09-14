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

    ssize_t Size() const;

    void Append(const char* _str, int _size);

    void Clear();
    
private:
    std::string buf_;
};