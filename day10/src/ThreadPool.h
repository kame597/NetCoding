#pragma once
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <vector>

class ThreadPool{
private:
    std::vector<std::thread> threads;        //存储所有线程
    std::queue<std::function<void()>> tasks; //任务队列，存储具体任务
    std::mutex tasks_mutex;                  //这个互斥锁是为了应付多线程下的读写操作
    std::condition_variable cv;              //条件变量，用于线程同步
    bool stop;                               //这个stop表示线程池是否关闭
public:
    ThreadPool(int size = 10);
    ~ThreadPool();

    void add(std::function<void()>);
};