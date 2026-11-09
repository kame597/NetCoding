#pragma once
#include "common.h"
#include <cstring>
// fixed_buffer是一个栈上的固定大小缓冲区，供LogStream拼接单条日志使用

const int kSmallBuffer = 4000;  // 全局常量，表示单条日志缓冲区的大小，即FixedBuffer的模版参数，定义在一处方便多处使用和修改
const int kLargeBuffer = 4000000;
const int kMaxNumericSize = 48; // 单个数字格式化可能占用的最大字节数

template<int SIZE>
class FixedBuffer{
public:
    DISALLOW_COPY_AND_MOVE(FixedBuffer);    // 此处禁用拷贝和移动，是因为防止cur_指针拷贝后仍指向原缓冲区
    FixedBuffer() : cur_(data_){}
    ~FixedBuffer() = default;

    /*
    *   @brief
    *   往缓冲区追加len字节的数据，空间不足时会丢弃本次的日志追加内容
    */
    void Append(const char* data, size_t len){
        if(AvailBytes() >= len){
            memcpy(cur_, data, len);
            cur_ += len;
        }
    }
    const char* data() const {return data_;}
    char* current() {return cur_;}  //返回当前可写位置

    size_t Length() const {return static_cast<size_t>(cur_ - data_);} //返回缓冲区已写入长度
    size_t AvailBytes() const {return sizeof(data_) - Length();}
    void Add(size_t len){cur_ += len;}

    void Reset(){cur_ = data_;}     // 重置可写指针，等效于重置缓冲区
    void Bzero(){memset(data_, 0, sizeof(data_));}   // 清空缓冲区内容


private:
    char data_[SIZE];   // 数据存储体
    char* cur_;         // 下一个可写位置
};

/*
*   data_                    cur_                 End()
*     │                        │                    │
*     ▼                        ▼                    ▼
*     ┌────────────────────────┬────────────────────┐
*     │   已写入 Length()       │  剩余 AvailBytes()  │
*     └────────────────────────┴────────────────────┘
*     │◄──────────────── SIZE ──────────────────────►│
*/
