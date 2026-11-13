#pragma once
#include "fixed_buffer.h"
#include "source_file.h"
#include "common.h"
#include "timestamp.h"

#include <cstdio>   // snprintf
#include <cstring>  // strlen
#include <string>
// log_stream是日志库的中间层，其职责为提供operator<<重载，把各种类型的值转成字节，追加到FixedBuffer
// 本质上是单一职责原则，LogStream只负责实现类型 -> 字节，供给Logger组装日志行
// 日志是高频路径，写在头文件便于

class LogStream{
public:
    DISALLOW_COPY_AND_MOVE(LogStream);
    using Buffer = FixedBuffer<kSmallBuffer>;

    LogStream() = default;
    ~LogStream() = default;

    const Buffer& buffer() const { return buffer_; }

    // 字符串处理
    LogStream& operator<<(const char* str){
        if(str != nullptr){    // 防止memcpy传入nullptr触发未定义行为
            buffer_.Append(str, strlen(str));   // 第二个参数为字符串长度
        }else{
            buffer_.Append("(null)", 6);
        }
        return *this;
    }

    LogStream& operator<<(const std::string& str){
        buffer_.Append(str.data(), str.size());     // 只记入有效字符，不会记入末尾的\0
        return *this;
    }


    // 字符处理
    LogStream& operator<<(char c){
        buffer_.Append(&c, 1);
        return *this;
    }


    // 整型处理
    LogStream& operator<<(short v){ return FormatNumber("%d", v); }
    LogStream& operator<<(int v){ return FormatNumber("%d", v); }
    LogStream& operator<<(long v){ return FormatNumber("%ld", v); }
    LogStream& operator<<(long long v){ return FormatNumber("%lld", v); }

    LogStream& operator<<(unsigned short v){ return FormatNumber("%u", v); }
    LogStream& operator<<(unsigned int v){ return FormatNumber("%u", v); }
    LogStream& operator<<(unsigned long v){ return FormatNumber("%lu", v); }
    LogStream& operator<<(unsigned long long v){ return FormatNumber("%llu", v); }

    // 浮点处理
    LogStream& operator<<(double v){ return FormatNumber("%.12g", v); }

    LogStream& operator<<(bool v){
        if(v){
            buffer_.Append("true", 4);
        }else{
            buffer_.Append("false", 5);
        }
        return *this;
    }

    LogStream& operator<<(const SourceFile& file){
        buffer_.Append(file.data(), file.size());
        return *this;
    }

    LogStream& operator<<(const TimeStamp& ts){
        char buf[64];
        size_t n = ts.FormatTo(buf, sizeof(buf));
        buffer_.Append(buf, n);
        return *this;
    }

    
private:
    // 用于序列化数字
    template<typename T>
    LogStream& FormatNumber(const char* fmt, T v){
        if(buffer_.AvailBytes() < static_cast<size_t>(kMaxNumericSize)) // 限制缓冲区预留空间为48字节，防止缓冲区溢出
            return *this;
        int n = snprintf(buffer_.current(), buffer_.AvailBytes(), fmt, v);  // snprintf：将v按照fmt格式化后，写到缓冲区buffer_.current()，最多写AvailBytes()，返回本该写入的字符数
        if(n > 0)
            buffer_.Add(static_cast<size_t>(n));
        return *this;
    }

    Buffer buffer_;
};
/*
* LOG_INFO << "fd=" << fd;
*   │
*   ▼
* Logger（组装时间/级别/文件名）
*   │
*   ▼
* LogStream::operator<< 
*   │
*   ▼
* FixedBuffer<4000>       ← 已完成
*/