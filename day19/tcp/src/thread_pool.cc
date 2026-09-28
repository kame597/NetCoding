#include "thread_pool.h"
#include <stdexcept>
#include <iostream>

ThreadPool::ThreadPool(int size):stop_(false){
    for(int i = 0; i < size; ++ i){
        threads_.emplace_back(std::thread([this, i]{ //此处要捕获this，因为接下来要用到类的成员变量
            while(true){
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(tasks_mutex_);
                    cv_.wait(lock, [this]{
                        //如果条件为真，则直接返回接着执行；否则释放锁，该线程等待
                        return (stop_ || !tasks_.empty());
                    });
                    if(stop_ && tasks_.empty()) return;
                    task = tasks_.front();   //如果任务队列不为空，则获取第一个任务，弹出任务队列
                    tasks_.pop();
                }
                task(); //执行任务
            }
        }));
    }
}

ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(tasks_mutex_);
        stop_ = true; //先上锁，修改线程池stop的值为true
    }
    cv_.notify_all(); //告知所有线程，stop的值已经修改为true;
    for(auto& th: threads_){
        if(th.joinable()){
            th.join();      //调用join，主线程等待子线程完成
        }
    }
}