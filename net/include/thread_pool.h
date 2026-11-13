#pragma once
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <vector>
#include <future>

class ThreadPool{
public:
    ThreadPool(int size = 10);
    ~ThreadPool();
    
    template<typename F, typename... Args>
    auto Add(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

private:
    std::vector<std::thread> threads_;        //存储所有线程
    std::queue<std::function<void()>> tasks_; //任务队列，存储具体任务
    std::mutex tasks_mutex_;                  //这个互斥锁是为了应付多线程下的读写操作
    std::condition_variable cv_;              //条件变量，用于线程同步
    std::atomic<bool> stop_{false};           //atomic：实现原子操作：读和写是最小操作，不会出现读一半被写的情况；线程能及时看到最新值
};

/*
*   @brief
*   向线程池的任务队列添加一个任务，并唤醒一个线程执行
*/
template<typename F, typename ...Args>
auto ThreadPool::Add(F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>>{

    using return_type = std::invoke_result_t<F, Args...>;
    
    /*
    *   forward是完美转发，减少了可调用对象的拷贝开销
    *   如果不用完美转发，即bind(f, args...); 因为f和args是具名变量，所以它们会被解析为左值
    *   bind会对左值执行拷贝操作，造成拷贝的开销
    *   而用完美转发，则根据具体类型推导，如果f是右值，则bind使用移动构造，避免了拷贝开销
    *   同理，参数包（假设是一个很大的vector）也可以用移动的方式避免拷贝开销
    */ 
    auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    /*
    *   task是一个指向packaged_task<>对象的共享指针
    *   packaged_task绑定了一个可调用对象func，其返回值为return_type()
    *   此处用共享指针，是因为task要装到任务队列里，而任务队列的类型为function
    *   function必须要求可拷贝，而packaged_task是不可拷贝的，因此用shared_ptr包装解决
    */
    auto task = std::make_shared<std::packaged_task<return_type()> >(func);

    /*
    *   get_future()是packaged_task<>对象的成员函数，用于返回future实例.
    *   其会修改packaged_task的内部状态，而线程执行task也会修改packaged_task的内部状态
    *   因此：需要在notify告知子线程做任务之前，保存一个future值，防止数据竞争（两个线程都访问packaged_task的内部）
    */
    std::future<return_type> res = task->get_future();

    {
        std::unique_lock<std::mutex> lock(tasks_mutex_);
        if(stop_){
            throw std::runtime_error("线程池已关闭！任务添加失败");
        }
        /*
        *   以lambda方式传入一个可调用对象
        *   该可调用对象捕获了task，并调用了task指向的packaged_lock
        *   由此绕过function的可拷贝要求
        */
        tasks_.emplace([task](){(*task)();});
    }

    cv_.notify_one();    //notify后，线程池就可以去执行任务队列里的任务了
    return res;
}