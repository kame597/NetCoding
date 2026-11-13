#pragma once
#include <mutex>
#include <condition_variable>

#include "common.h"

/*
*   背景：
*   多线程环境中，常有这样一种同步情况：一个线程等待其他所有线程完成指定工作。
*   比如，在开启一个新线程后，虽然已经启动线程函数，很可能还有一些准备工作需要完成（如更新当前线程id，名称等信息），
*   这样，调用线程（创建线程的线程）并不能马上投入工作，立即向新线程传递数据，可能造成未定义行为，如调用线程的某段代码依赖于子线程id。
*/

/*
*   用于多个线程之间的同步，特别是一个线程等待另一个或多个线程。
*   内部持有一个向下计数的计数器count_，构造时给定一个初值，代表需要等待的线程数。
*   每个线程完成一个任务，count_减1，当count_值减到0时，代表所有线程已经完成了所有任务，在CountDownLatch上等待的线程就可以继续执行了。
*/
class Latch{
public:
    DISALLOW_COPY_AND_MOVE(Latch);
    explicit Latch(int count) : count_(count){}

    ~Latch(){};

    void Wait(){
        std::unique_lock lock(mutex_);
        cond_.wait(lock, [this](){
            return count_ == 0; 
        });
    }

    void CountDown(){
        std::lock_guard<std::mutex> lock(mutex_);
        if(count_ > 0){
            -- count_;
            if(count_ == 0)
                cond_.notify_all();     // 此处实际上只是唤醒主线程，CountDown由后端线程调用
        }
    }

    int get_count() const {
        std::lock_guard lock(mutex_);   // 此处如果mutex不是mutable，则绑定的就是const std::mutex
        return count_;
    }

private:
    mutable std::mutex mutex_;  // mutable，表示“这个成员的修改不算改变对象的逻辑状态”，即在const函数内允许改变
    std::condition_variable cond_;
    int count_;     // 表示需要等待的线程数
};