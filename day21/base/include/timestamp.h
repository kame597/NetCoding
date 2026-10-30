#pragma once
#include <sys/time.h>
#include <cstdint>
#include <string>

/*
*   我们将TimeStamp放在base
*   因为TimeStamp不依赖上层的tcp和http等
*/

/*
*   @brief
*   TimeStamp 用于表示时间点，内部是自 Unix 纪元以来的微秒数。
*   使用 CLOCK_REALTIME（gettimeofday），即真实时间，用于日志记录。
*/
class TimeStamp{
public:
    TimeStamp();
    ~TimeStamp();
    explicit TimeStamp(int64_t micro_seconds);  //根据微秒数构造

    bool operator<(const TimeStamp& rhs) const;     // todo 运算符重载
    bool operator==(const TimeStamp& rhs) const;    // 此处运算符重载是为了定时器排序

    // 此处AddTime和Now需要直接通过TimeStamp::func()的方式静态调用，因此需要static关键字
    // 另外，设置static还因为这两个函数不需要用到this指针
    static TimeStamp AddTime(const TimeStamp& timestamp, double add_seconds);
    static TimeStamp Now();
    std::string ToFormattedString() const;
    int64_t microseconds() const;

private:
    int64_t micro_seconds_;     // 微秒数，用int64_t可以直接to_string用于日志，且可以跨平台统一
    // micro_seconds_指的是自"1970-01-01 00:00:00 UTC"（Unix纪元）起，经过了多少微秒数
};