#include "buffer.h"
#include <iostream>

void Buffer::Append(const char* _str, size_t _size){
    buf_.append(_str, _size);
}

ssize_t Buffer::Size() const{
    return buf_.size();
}

const char* Buffer::c_str() const{
    return buf_.c_str();
}

void Buffer::Clear(){
    buf_.clear();   
}

const char *Buffer::FindCRLF() const
{
    size_t pos = buf_.find("\r\n");
    return pos == std::string::npos ? nullptr : buf_.data() + pos;
}

/*
setBuf函数
用于设定Buffer的内容
*/
void Buffer::set_buf(const char* buf){
    buf_.clear();
    buf_.append(buf);
}

void Buffer::set_buf(const std::string& buf){
    buf_ = buf;
}
