#include "timer_queue.h"
#include "event_loop.h"
#include "channel.h"
#include <unistd.h>


TimerQueue::TimerQueue(EventLoop* loop) :
    loop_(loop),
    // TFD_NONBLOCK —— 非阻塞模式,该fd不会阻塞
    // TFD_CLOEXEC —— 执行 exec 时自动关闭，不会泄露给新程序
    timerfd_(timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC)),
    timer_channel_(std::make_unique<Channel>(timerfd_, loop)){
        timer_channel_->set_read_callback(std::bind(&TimerQueue::HandleRead, this));
        timer_channel_->EnableRead();
}

TimerQueue::~TimerQueue(){
    if (timerfd_ != -1){    // 直接通过loop删除该channel。因为timer_channel是单独channel，不与
        loop_->DeleteChannel(timer_channel_.get());
    }
}

/*
*   @brief
*   根据传入任务与时间戳构造TimerEntry，插入表中管理，并重置更新timerfd_
*/
int64_t TimerQueue::AddTimer(TimerCallback cb, TimeStamp timestamp, double interval){
    int64_t id = next_id_ ++;   
    TimerEntry entry{id, std::move(cb), timestamp, interval};
    
    auto it = timer_set_.insert(entry).first;   // 依赖set的排序性，判断插入的定时器是否是最早的，如果是，则更新timerfd
    if(it == timer_set_.begin()) 
        ResetTimerfd(timestamp);
    
    return id;  //返回添加的timer的id
}

/*
*   @brief
*   删除一个定时器任务，并依情况更新timerfd_
*/
void TimerQueue::CancelTimer(int64_t id){
    for(auto it = timer_set_.begin(); it != timer_set_.end(); ++ it){
        if(it->id == id){
            bool is_earliest = (it == timer_set_.begin());
            timer_set_.erase(it);
            if(is_earliest && !timer_set_.empty()){     //如果删除的定时器任务是最早的，且任务集里还有定时任务，则更新timerfd_
                ResetTimerfd(timer_set_.begin()->time);
            }else{
                struct itimerspec spec{};
                timerfd_settime(timerfd_, 0, &spec, nullptr);   //解除timerfd_，避免空触发（当timerfd设定时间为0时，则timerfd失效）
            }
            return;
        }
    }
}

/*
*   @brief
*   获取到期定时器并返回，清理定时器表
*/
std::vector<TimerEntry> TimerQueue::GetExpired(TimeStamp now)
{
    auto end = timer_set_.upper_bound(TimerEntry{INT64_MAX, nullptr, now, 0});
    std::vector<TimerEntry> res(timer_set_.begin(), end);
    timer_set_.erase(timer_set_.begin(), end);
    return res;
}

/*
*   @brief
*   timerfd的回调。获取已到期的定时任务并执行其绑定回调
*/
void TimerQueue::HandleRead(){
    uint64_t exp;
    // 返回值n表示实际读到的字节数。timerfd到期后，会把到期次数写成一个8字节的uint64，因此成功时n = 8(sizeof(exp))
    ssize_t n = read(timerfd_, &exp, sizeof(exp));  
    if(n != sizeof(exp))
        return;
    TimeStamp now = TimeStamp::Now();
    auto expired = GetExpired(now);

    for(auto& entry : expired){
        entry.cb();
        if(entry.interval > 0){
            entry.time = TimeStamp::AddTime(TimeStamp::Now(), entry.interval);
            timer_set_.insert(entry);   // 因为GetExpired会清除定时器，因此此处要回插
        }
    }

    if(!timer_set_.empty())
        ResetTimerfd(timer_set_.begin()->time);
}

/*
 *   @brief
 *   重置更新timerfd_，指向最新的到期任务
 */
void TimerQueue::ResetTimerfd(TimeStamp timestamp){     //todo 应该加入interval的更新，当前版本先假定interval为0
    int64_t micros = timestamp.microseconds() - TimeStamp::Now().microseconds();
    struct itimerspec spec{};

    // 保护措施，防止插入了过去的时间，用于预防一个定时器任务已过期但未被处理
    if(micros <= 0){
        spec.it_interval.tv_sec = 0;
        spec.it_value.tv_sec = 0;
        spec.it_value.tv_nsec = 1;  // 对于已过期任务，立即执行
    }
    else{
        spec.it_interval.tv_sec = 0;
        spec.it_value.tv_sec = micros / 1000000;           // 微秒换算为秒
        spec.it_value.tv_nsec = (micros % 1000000) * 1000; // 乘1000是将微秒切换为纳秒
    }
    timerfd_settime(timerfd_, 0, &spec, nullptr);
}
