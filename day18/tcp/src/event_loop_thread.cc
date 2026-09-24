#include "event_loop_thread.h"
#include "event_loop.h"

EventLoopThread::EventLoopThread():created_loop_(nullptr){}

EventLoopThread::~EventLoopThread(){
    if(thread_.joinable())
    /*
    *  暂时的析构方法，放弃等待，让线程自生自灭
    *  如果是join，则是等待线程返回，但是EventLoop是一个死循环，因此会阻塞
    */
        thread_.detach(); 
        
}

EventLoop *EventLoopThread::StartLoopThread()
{   
    thread_ = std::thread(&EventLoopThread::ThreadFunc, this);
    std::unique_lock<std::mutex> lock(mutex_);    
    cv_.wait(lock,[this]{   //此处cv是为了解决同步问题，防止此时的created_loop_还没被创建
        /*
        *   如果条件为真，则直接返回接着执行；
        *   如果条件为假，则释放锁，该线程等待
        */
        return created_loop_ != nullptr;
    });
    return created_loop_;
}

/*
*   ThreadFunc
*   @brief
*   创建一个EventLoop对象，用created_loop_获取该对象的地址，执行Loop
*   此处上锁是因为主线程在StartLoopThread中需要返回created_loop_的值，存在数据竞争
*/
void EventLoopThread::ThreadFunc()
{
    EventLoop loop; //栈上局部对象，真正本体
    {
        std::unique_lock<std::mutex> lock(mutex_);
        created_loop_ = &loop;  //成员指针，指向本体
        cv_.notify_one();
    }
    created_loop_ -> Loop();
    {
        std::unique_lock<std::mutex> lock(mutex_);
        created_loop_ = nullptr;
    }
}
