#include "latch.h"
#include "logger.h"
#include <thread>
#include <functional>

namespace
{
    Latch latch(10);
}

int main(){
    
    std::thread thread1([]{
        LOG_DEBUG << "thread1 调用 latch.wait";
        latch.Wait();
        LOG_DEBUG << "thread1 阻塞结束";
    });

    std::thread thread2([]{
        LOG_DEBUG << "thread2 调用 latch.countdown";
        while(true){
            latch.CountDown();
        }
    });

    thread1.join();
    thread2.join();

    return 0;
}

/*
*   复习一下thread
*   thread创建了一个对象t，t本质上是一个句柄，管理一个后台线程A
*   对象t有自身的析构函数，意味着其会在main结束后析构
*   如果对象t析构时，其未被join或detach，则t的析构函数就会抛出异常
*   因此，需要在创建对象t的作用域内，调用join或detach，来保障不会抛出异常
*
*   join的语义是调用方(main)阻塞，等待t的任务完成，并使t可以安全析构
*   detach的语义是调用方不阻塞，t解除管理的线程A，线程A由系统自动回收。在解除管理后，如果线程A会访问调用方的变量，则会导致悬空引用错误
*/