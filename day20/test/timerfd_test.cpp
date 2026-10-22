#include <sys/timerfd.h>
#include "timer_queue.h"
#include <memory>
#include <unistd.h>     // read
#include <cstdio>
#include <cstdint>

int main(){

    /*
    *   struct itimerspec{
    *     struct timespec it_interval;  // 周期（重复的间隔），表示之后每隔多久重复一次，设置为0表示只触发一次
    *     struct timespec it_value;     // 首次到期时间  
    *   };
    * 
    *   struct timespec{
    *       time_t tv_sec;  //秒
    *       long tv_nsec;   //纳秒
    *   };
    */

    std::unique_ptr<TimerQueue> timer_queue = std::make_unique<TimerQueue>();
    
    int fd = timerfd_create(CLOCK_REALTIME, 0);     // 创建timerfd
    struct itimerspec spec{};
    spec.it_interval.tv_sec = 0;
    spec.it_value.tv_sec = 3;                       // 设置首次到期时间为3秒
    
    /* 
    *   int timerfd_settime(int fd, int flags,
    *               const struct itimerspec *new_value,
    *               struct itimerspec *old_value);
    */
    timerfd_settime(fd, 0, &spec, nullptr);         // 将spec里的时间参数，装载进fd对应的内核对象的定时器，内核维护定时器，到期时将fd变为可读
                                                    // flag设为0，表示相对时间，即从现在开始3秒后
                                                    // old_value传nullptr表示不关心旧值；
    uint64_t exp;
    read(fd, &exp, sizeof(exp));                    // 阻塞read等待，对于timerfd，如果定时器没到期，则read会阻塞，除非fd设置为非阻塞
    printf("时间到! count = %lu\n", exp);
    close(fd);
}