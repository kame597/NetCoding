#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include "src/ThreadPool.h"

std::mutex task_mutex;

void print(int a, double b, const char* c, std::string d){
    std::lock_guard lock(task_mutex);
    std::cout << a << b << c << d << std::endl;
}

void test(){
    std::lock_guard lock(task_mutex);
    std::cout << "test" << std::endl;
}

int main(int argc, const char* argv[]){   
    std::unique_ptr<ThreadPool> poll = std::make_unique<ThreadPool>();  //新建一个线程池，默认分配10个线程
    std::function<void()> func = std::bind(print, 1, 3.14, "hello", std::string("world"));
    poll -> add(func);
    func = test;
    poll -> add(func);
    return 0;
}
