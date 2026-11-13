#pragma once
// LogLevel单独抽出：“级别”这个枚举被三处（Logger构造参数，宏过滤判断，输出时转文本）用到，需要抽出来单独成类
// 回答两个问题：日志的严重程度，和日志严重程度文本打印

enum class LogLevel{
    kDebug = 0, kInfo, kWarn, kError, kFatal,   //k表示常量，此处表编译器常量
};

inline const char* level_to_string(LogLevel level){
// 注：该函数设定为内联，是因为如果不用内联，会导致所有#include该文件的文件，都会生成一份level_to_string定义，爆出重定义问题
// 用内联可以保证多个编译单元出现相同的inline函数定义，编译器只会保留一份，符合ODR（单一定义原则）
// 注：该函数返回类型为const char*而非string，是因为const char*是字符串字面量地址，返回一个指针不会构造对象，不会分配内存，所以是零开销
// 如果返回string，则每次调用都要在堆上分配内存，拷贝字符，调用方用完还要析构，造成性能开销
    switch (level)
    {
    case LogLevel::kDebug: return "DEBUG";
    case LogLevel::kInfo:  return "INFO";
    case LogLevel::kWarn:  return "WARN";
    case LogLevel::kError: return "ERROR";
    case LogLevel::kFatal: return "FATAL";
    default:    return "UNKNOWN";
    }
}
