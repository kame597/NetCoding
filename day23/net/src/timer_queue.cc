#include "timer_queue.h"
#include "event_loop.h"
#include "channel.h"
#include <unistd.h>
#include "utils.h"
#include "iostream"


TimerQueue::TimerQueue(EventLoop* loop) :
    loop_(loop),
    // TFD_NONBLOCK —— 非阻塞模式,该fd不会阻塞
    // TFD_CLOEXEC —— 执行 exec 时自动关闭，不会泄露给新程序
    timerfd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
    timer_channel_(std::make_unique<Channel>(timerfd_, loop)){
        errif(timerfd_ == -1, "timerfd创建失败");
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
    id_to_time_[id] = timestamp;

    // std::cout << "[AddTimer]定时任务已插入" << std::endl;
    if(it == timer_set_.begin()) {
        ResetTimerfd(timestamp);
        // std::cout << "[AddTimer]定时器已重置" << std::endl;
    }
    
    return id;  //返回添加的timer的id
}

/*
*   @brief
*   删除一个定时器任务，并依情况更新timerfd_
*/
void TimerQueue::CancelTimer(int64_t id){
    /*
        根据id_to_time_找到需要取消的定时器对应的时间，
        根据时间构造TimerEntry供timer_set_的find查找，以此避免线性查找
    */ 
        auto map_it = id_to_time_.find(id);
        if(map_it == id_to_time_.end()) return;
        auto it = timer_set_.find(TimerEntry{id, nullptr, map_it->second, 0});
        if(it == timer_set_.end()){
            id_to_time_.erase(map_it);
            return;
        }
        bool is_earliest = (it == timer_set_.begin());
        timer_set_.erase(it);
        id_to_time_.erase(map_it);

        if(!timer_set_.empty()){     //如果删除的定时器任务是最早的，且任务集里还有定时任务，则更新timerfd_
            if(is_earliest)
                ResetTimerfd(timer_set_.begin()->time);
        }else{  // 如果删除后定时器表为空，则解除timerfd
            struct itimerspec spec{};
            timerfd_settime(timerfd_, 0, &spec, nullptr);   //解除timerfd_，避免空触发（当timerfd设定时间为0时，则timerfd失效）
        }
        return;
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
    // std::cout << "[HandleRead]已执行" << std::endl;
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
            entry.time = TimeStamp::AddTime(now, entry.interval);
            timer_set_.insert(entry);   // 因为GetExpired会清除定时器，因此此处要回插
            id_to_time_[entry.id] = entry.time;
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
    timerfd_settime(timerfd_, 0, &spec, nullptr);   // 注：flag为0，表示用的是相对时间
    // std::cout << "[ResetTimerfd]timerfd已更新" << std::endl;
}
