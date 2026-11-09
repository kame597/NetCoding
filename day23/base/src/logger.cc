#include "logger.h"
#include "timestamp.h"
#include "current_thread.h"
#include "async_logging.h"

#include <cstdio>  // fwrite, fflush, stdout; fwrite是C标准输出，在用户态维持一个4096字节缓冲区，会将数据拷贝到用户态缓冲区，直到满了才调用write真正写入，返回值为实际写入元素个数
#include <cstdlib>  // abort

namespace{

    std::string g_log_basename;
    int g_flush_interval = 3;

    // 返回全局变量instance，用于异步日志库调用，所有Logger都通过该实例
    AsyncLogging& AsyncLogger(){
        static AsyncLogging instance(g_log_basename, g_flush_interval);
        return instance;
    }

    // 返回全局变量minLevel，表示允许输出的最低日志等级
    LogLevel& LogLevelRef(){
        // 注：此处用一个函数包装是为了避免连接先后顺序，导致日志的等级筛选失效，实际上只用全局变量一般是可行的
        static LogLevel minLevel = LogLevel::kDebug;    //默认设置为Debug，代表所有等级都允许
        return minLevel;
    }
    void DefaultOutput(const char* msg, size_t len){ fwrite(msg, 1, len, stdout); }    // fwrite第一个参数msg表示要写数据的指针，第二个参数表示一个元素多少字节，第三个参数len表示写入多少个元素，第4个参数表示要写入的目标文件流
    void DefaultFlush(){ fflush(stdout); }  // fflush将输入的FILE*流的内容强制刷出
}

/*
*   @brief
*   构造时存下传入的文件名，行号，日志等级，用于后续拼接日志使用
*/
Logger::Logger(const char *file, int line, LogLevel level):
    level_(level), line_(line), source_file_(file)
{
    FormatPrefix();
}

/*
*   @brief
*   Logger析构时，才将缓冲区的内容输出到指定的*FILE缓冲区
*/
Logger::~Logger()
{
    stream_ << '\n';
    const auto& buf = stream_.buffer();

    if(level_ == LogLevel::kFatal){
        DefaultOutput(buf.data(), buf.Length());
        DefaultFlush();  // 用flush刷新缓冲区防止丢失日志
        abort();    // 如果爆出Fatal应该立即中断程序
    }
     AsyncLogger().Append(buf.data(), buf.Length());
}

bool Logger::ShouldLog(LogLevel level)
{
    // 注：ShouldLog使用static关键字，为了能够在宏内直接调用
    return level >= LogLevelRef();  
}

void Logger::SetLogLevel(LogLevel level){
    LogLevelRef() = level;
}

LogLevel Logger::GetLogLevel()
{
    return LogLevelRef();
}

void Logger::SetLogFile(std::string basename)
{
    g_log_basename = basename;
}

void Logger::SetInterval(int interval)
{
    g_flush_interval = interval;
}

/*
 *   @brief
 *   生成标准化的日志前缀
 */
void Logger::FormatPrefix(){
    stream_ << '[';
    stream_ << level_to_string(level_); 
    stream_ << ']';
    stream_ << ' ';
    stream_ << TimeStamp::Now();    // 当前时间
    stream_ << ' ';
    stream_ << current_thread::tid_string();    // 当前线程id
    stream_ << ' ';
    stream_ << source_file_;
    stream_ << '-';
    stream_ << line_;
    stream_ << ':';     // 随后接日志正文
    stream_ << ' ';
}

