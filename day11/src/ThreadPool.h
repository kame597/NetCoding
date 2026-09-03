#pragma once
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <vector>
#include <future>

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

    // void add(std::function<void()>);

    //TODO 模版编程？？？future？？？
    template<class F, class ...Args>
    //auto add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    //注：result_of在C++17被弃用了，invoke_result_t是现代写法，其不依赖具体的变量名，只需要提供类型即可推断
    auto add(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;
};


//模版编程的定义不能放在cpp文件，原因是C++编译器不支持模版的分离编译
template<class F, class ...Args>
auto ThreadPool::add(F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>>{
    using return_type = std::invoke_result_t<F, Args...>;
    
    //TODO packaged_task是什么鸡毛？？为什么这里要开共享指针？？
    //TODO forward是完美转发吗？...是某种特殊运算符？？
    auto task = std::make_shared<std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    //TODO future到底是什么玩意🤔,get_future是哪来的
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(tasks_mutex);
        if(stop){
            throw std::runtime_error("线程池已关闭！任务添加失败");
        }
        tasks.emplace([task](){(*task)();});
    }
    cv.notify_one();
    return res;
}