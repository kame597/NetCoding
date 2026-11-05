#pragma once
#include "log_level.h"
#include "log_stream.h"
#include "source_file.h"
#include "common.h"

/*
*   Logger是日志的组装和输出器，其工作为：
*   组装日志前缀，包括时间戳，日志级别，线程id，文件名，行号
*   持有LogStream，让用户持续追加内容
*   析构时输出，将缓冲区内容一次性fwrite写出，补上\n
*
*   Logger需要利用析构函数自动输出，方便宏写成 "LOG_INFO << "fd=" << fd;"的形式
*   Logger是一次性的，用operator<<写入正文，析构时输出
*/ 
class Logger{
public:
    DISALLOW_COPY_AND_MOVE(Logger);
    using OutputFunc = void(*)(const char* msg, size_t len);
    using FlushFunc = void(*)();    // flush指的是将缓冲区没写出的数据强制推出 (因为fwrite只在缓冲区满了才发送)

    Logger(const char* file, int line, LogLevel level);
    ~Logger();

    LogStream& stream(){ return stream_; }    // 返回内部LogStream&

    static bool ShouldLog(LogLevel level);  

    // 设置允许输出的最低日志等级
    static void SetLogLevel(LogLevel level);
    static LogLevel GetLogLevel();

    // 设置输出区域和刷新区域，在打印第一条日志之前调用
    static void SetOutput(OutputFunc func); 
    static void SetFlush(FlushFunc func);

    

private:
    void FormatPrefix();
    LogStream stream_;
    LogLevel level_;
    int line_;
    SourceFile source_file_;
};

#define LOG_DEBUG \
    if(Logger::ShouldLog(LogLevel::kDebug))\
    Logger(__FILE__, __LINE__, LogLevel::kDebug).stream()

#define LOG_INFO \
    if(Logger::ShouldLog(LogLevel::kInfo))\
    Logger(__FILE__, __LINE__, LogLevel::kInfo).stream() 

#define LOG_WARN \
    if(Logger::ShouldLog(LogLevel::kWarn))\
    Logger(__FILE__, __LINE__, LogLevel::kWarn).stream() 

#define LOG_ERROR \
    if(Logger::ShouldLog(LogLevel::kError))\
    Logger(__FILE__, __LINE__, LogLevel::kError).stream() 

#define LOG_FATAL \
    if(Logger::ShouldLog(LogLevel::kFatal))\
    Logger(__FILE__, __LINE__, LogLevel::kFatal).stream()


/*
*   最终要达成的效果：
*   业务代码写：LOG_INFO << "fd=" << fd;
*   屏幕上出现信息：20261102 10:23:45.123456 12345 INFO  tcp_connection.cc:152 fd=8
*
*   业务代码用宏展开，比如#define LOG_INFO(msg) printf("%s:%d %s\n", __FILE__, __LINE__, msg)
*   业务代码 LOG_INFO("fd=8")
*   展开成 printf("%s:%d %s\n", "tcp_connection.cc", 152, "fd=8")   ✅
*   理由是宏展开能获取调用点的__FILE__和__LINE__，而如果写在函数调用里则只会获取Logger的文件名和行
*
*   注：*FILE缓冲区只有在abort，段错误，直接杀进程的情况下才会丢日志，其他情况下就算*FILE没填满，其也会在程序正常退出时写出
*   
*/