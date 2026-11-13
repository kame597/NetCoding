#include "logger.h"
#include <thread>

int main(){
    Logger::SetLogFile("log.txt");
    Logger::SetInterval(5);
    Logger::SetLogLevel(LogLevel::kDebug);

    std::thread thread1([]{
        LOG_DEBUG << "你好吗?";
        LOG_WARN << "我不好……";
    });
    
    std::thread thread2([]{
        LOG_INFO << "真是遗憾";
        LOG_ERROR << "希望你能好XD";
    });
    
    thread1.join();
    thread2.join();
}