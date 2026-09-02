#include "ThreadPool.h"
#include <stdexcept>
#include <iostream>

ThreadPool::ThreadPool(int size):stop(false){
    for(int i = 0; i < size; ++ i){
        threads.emplace_back(std::thread([this, i]{ //此处要捕获this，因为接下来要用到类的成员变量
            while(true){
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(tasks_mutex);
                    cv.wait(lock, [this]{
                        //如果条件为真，则直接返回接着执行；否则释放锁，该线程等待
                        return (stop || !tasks.empty());
                    });
                    if(stop && tasks.empty()) return;
                    task = tasks.front();   //如果任务队列不为空，则获取第一个任务，弹出任务队列
                    tasks.pop();
                }
                task(); //执行任务
            }
        }));
    }
}

ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(tasks_mutex);
        stop = true; //先上锁，修改线程池stop的值为true
    }
    cv.notify_all(); //告知所有线程，stop的值已经修改为true;
    for(auto& th: threads){
        if(th.joinable()){
            th.join();      //调用join，主线程等待子线程完成
        }
    }
}

void ThreadPool::add(std::function<void()> func){
    {
        std::unique_lock<std::mutex> lock(tasks_mutex);
        if(stop)
            throw std::runtime_error("错误! 线程池已经停止! 无法添加新任务");
        tasks.emplace(func);
    }
    cv.notify_one(); //告知等待线程中的其中一个线程：有任务到了，起来干活
}