#include "timestamp.h"
#include <ctime>    // localtime_r, struct tm
#include <cstdio>   // snprintf

TimeStamp::TimeStamp():micro_seconds_(0){
}

TimeStamp::TimeStamp(int64_t micro_seconds) : micro_seconds_(micro_seconds)
{}

bool TimeStamp::operator<(const TimeStamp &rhs) const{
    return this->micro_seconds_ < rhs.micro_seconds_;
}

bool TimeStamp::operator==(const TimeStamp &rhs) const{
    return this->micro_seconds_ == rhs.micro_seconds_;
}

/*
*   @brief
*   接受一个时间戳和一个时间差值
*   返回该加上该差值的时间戳
*/
TimeStamp TimeStamp::AddTime(const TimeStamp &timestamp, double add_seconds){
    int64_t delta_time = static_cast<int64_t>(add_seconds * 1000000);
    return TimeStamp(timestamp.microseconds() + delta_time);
}

/*
*   @brief
*   获取当前真实事件，返回当前时间戳
*/
TimeStamp TimeStamp::Now(){
    /*
    *   struct timeval
    *   time_t tv_sec;  自1970纪元时间起的秒数
    *   suseconds_t tv_usec 秒内小数部分
    * 
    */
    struct timeval tv;
    gettimeofday(&tv, nullptr);     //gettimeofday用于获取真实时间，将真实时间写入tv中
    int64_t micro_seconds = static_cast<int64_t>(tv.tv_sec) * 1000000 + static_cast<int64_t>(tv.tv_usec);   //将秒转化为微秒，再拼上小数
    return TimeStamp(micro_seconds);
}

std::string TimeStamp::ToFormattedString() const{
    char buf[64];   // 这个64只是一个经验参考值
    time_t seconds = static_cast<time_t>(micro_seconds_ / 1000000);     // 将微秒单位切换为秒单位
    /*
    *  struct tm
    *   {
    *   int tm_sec;			Seconds.[0-60] (1 leap second)
    *   int tm_min;			Minutes.[0-59]
    *   int tm_hour;	    Hours.	[0-23]
    *   int tm_mday;		Day.	[1-31]
    *   int tm_mon;			Month.	[0-11]
    *   int tm_year;		Year	- 1900.
    *   int tm_wday;		Day of week.[0-6]
    *   int tm_yday;		Days in year.[0-365]
    *   int tm_isdst;	    DST.		[-1/0/1]
    */
    struct tm tm_time;

    //  接收一个时间点（从纪元时间到当前时间经过的秒数），将它翻译成本地时区下的日历字段，存入tm_time
    localtime_r(&seconds, &tm_time);    

    // 输出：YYYY-MM-DD HH:MM:SS.XXXXXX
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%06ld",
        tm_time.tm_year + 1900,
        tm_time.tm_mon + 1,
        tm_time.tm_mday,
        tm_time.tm_hour,
        tm_time.tm_min,
        tm_time.tm_sec,
        static_cast<long>(micro_seconds_ % 1000000)
    );
    return std::string(buf);
}

int64_t TimeStamp::microseconds() const{
    return micro_seconds_;
}
