#include "event_loop.h"
#include <functional>
#include <iostream>
#include <memory>

void TestOneShot(){
    std::cout << "[OneShot]已执行" << std::endl;
}

void TestLoop(unsigned& count){
    ++ count;
    std::cout << "[Loop]已执行" << count  << "次" << std::endl;
}

int main(){
    std::cout << "项目开始" << std::endl;
    std::unique_ptr<EventLoop> loop = std::make_unique<EventLoop>();
    std::cout << "执行第一个定时任务" << std::endl;
    auto id1 = loop->RunAfter(10, TestOneShot);
    unsigned count = 0;
    std::cout << "执行第二个定时任务" << std::endl;
    auto id2 = loop->RunEvery(5, std::bind(TestLoop, count));
    loop->RunAfter(20, std::bind(&EventLoop::CancelTimer, loop.get(),id2));
    loop->Loop();
}