#include "buffer.h"
#include <iostream>

void Buffer::Append(const char* _str, int _size){
    for(int i = 0; i < _size; ++ i){
        if(_str[i] == '\0') break;
        buf_.push_back(_str[i]);
    }
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

/*
setBuf函数
用于设定Buffer的内容
*/
void Buffer::set_buf(const char* buf){
    buf_.clear();
    buf_.append(buf);
}