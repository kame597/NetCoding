#pragma once
#include "timestamp.h"
#include "common.h"

#include <sys/timerfd.h>
#include <set>
#include <cstdint>
#include <functional>
#include <memory>

class EventLoop;
class Channel;

struct TimerEntry{
    int64_t id;
    std::function<void()> cb;
    TimeStamp time;
    double interval;    //如果不是只触发一次时，设定的周期时间

    bool operator<(const TimerEntry& rhs) const{
        if(this->time == rhs.time)
            return this->id < rhs.id; 
        else    
            return this->time < rhs.time;
    }
};

/*
*   定时器调度管理器，用于管理TimerEntry，并驱动相关操作
*/
class TimerQueue{
public:
    using TimerCallback = std::function<void()>;
    DISALLOW_COPY_AND_MOVE(TimerQueue);

    
    
    // 启动，构造timerfd_
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    // 根据传入任务与时间戳构造TimerEntry，插入表中管理，并重置更新timerfd_
    int64_t AddTimer(TimerCallback cb, TimeStamp timestamp, double interval = 0);    

    // 删除一个定时器任务，并依情况更新timerfd_
    void CancelTimer(int64_t id);

    // 获取到期的定时器
    std::vector<TimerEntry> GetExpired(TimeStamp now);

private:
    void HandleRead();
    void ResetTimerfd(TimeStamp timestamp);
    EventLoop* loop_;
    int timerfd_;               //  timerfd, 用于唤醒定时就绪事件
    std::unique_ptr<Channel> timer_channel_;
    std::set<TimerEntry, std::less<TimerEntry>> timer_set_;     //升序set
    int64_t next_id_ = 0;       //  next_id_, 用于赋给TimerEntryid
    std::unordered_map<int64_t, TimeStamp> id_to_time_;     // id_to_time_，用于在CancelTimer时降低时间复杂度
};

/*
*   拟定：
*   // 注册一个定时器
*   timerQueue.addTimer([]() {
*    // 这里写"要做什么"
*    connection->shutdown();
*   }, TimeStamp::AddTime(TimeStamp::Now(), 60.0), 0);
*/