#include "logger.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include <atomic>

/*
*   关于命名空间：
*       1. 命名空间核心作用，是让其中定义的名字只在当前翻译单元（.cpp 文件）内可见
*       2. 对于编译器而言，声明匿名命名空间，等价于为该翻译单元生成一个独一的名字，并隐式地using namespace
*       3. 命名空间的名字都是内部链接，意味着只在当前这个 .cpp 文件内有效，不会导致文件间定义冲突
*       4. 命名空间跟static全局变量用法相似，但命名空间内可以包含更多元素，比如类，而static则不行
*/
namespace{
    FILE* g_log_fp = nullptr;
    std::atomic<int> g_output_calls{0};
}

/*
*   @brief
*   用例1：级别过滤
*   设置 kWarn 后，DEBUG / INFO 不应输出，WARN / ERROR 应输出
*/
static void TestLevelFilter(){
    printf("--- TestLevelFilter ---\n");
    Logger::SetLogLevel(LogLevel::kWarn);
    LOG_DEBUG << "SHOULD_NOT_APPEAR_debug";
    LOG_INFO  << "SHOULD_NOT_APPEAR_info";
    LOG_WARN  << "SHOULD_APPEAR_warn";
    LOG_ERROR << "SHOULD_APPEAR_error";
    Logger::SetLogLevel(LogLevel::kDebug);
}

/*
*   @brief
*   用例2：自定义输出被调用次数
*   每写一条日志，OutputFunc 应被调用恰好一次
*/
static void TestCustomOutput(){
    printf("--- TestCustomOutput ---\n");
    g_output_calls = 0;
    Logger::SetOutput([](const char* msg, size_t len){
        (void)msg; (void)len;
        g_output_calls.fetch_add(1, std::memory_order_relaxed);
    });

    const int kLines = 100;
    for(int i = 0; i < kLines; ++i){
        LOG_INFO << "custom output " << i;
    }

    int calls = g_output_calls.load(std::memory_order_relaxed);
    printf("expected %d calls, got %d -> %s\n", kLines, calls, calls == kLines ? "OK" : "FAIL");
    if(calls != kLines){
        fprintf(stderr, "TestCustomOutput FAILED\n");
        exit(EXIT_FAILURE);
    }
}

/*
*   @brief
*   用例3：多线程输出不交错
*   8 线程各写 10000 条，每行必须完整（以 '[' 开头、以 '\n' 结尾），
*   且总行数必须等于 80000。任何一行被截断或拼接都会导致行数/格式不符。
*/
static void TestMultiThreadNoInterleave(){
    printf("--- TestMultiThreadNoInterleave ---\n");

    const char* path = "log_multithread.txt";
    FILE* fp = fopen(path, "w+b");
    if(fp == nullptr){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    Logger::SetOutput([](const char* msg, size_t len){
        fwrite(msg, 1, len, g_log_fp);
    });
    g_log_fp = fp;

    const int kThreads = 8;
    const int kPerThread = 10000;
    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for(int t = 0; t < kThreads; ++t){
        threads.emplace_back([t]{
            for(int i = 0; i < kPerThread; ++i){
                LOG_INFO << "T" << t << " seq=" << i;
            }
        });
    }
    for(auto& th : threads) th.join();

    fflush(fp);
    fclose(fp);
    g_log_fp = nullptr;

    // 校验：逐行读取，检查行数与每行格式
    fp = fopen(path, "rb");
    if(fp == nullptr){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    std::string line;
    long total = 0;
    long bad = 0;
    int ch;
    while((ch = fgetc(fp)) != EOF){
        if(ch == '\n'){
            ++total;
            // 合法行：以 '[' 开头，且包含 " seq=" 与 "T<数字>"
            if(line.empty() || line[0] != '[' || line.find(" seq=") == std::string::npos){
                ++bad;
                if(bad <= 3){
                    fprintf(stderr, "bad line: %s\n", line.c_str());
                }
            }
            line.clear();
        }else{
            line.push_back(static_cast<char>(ch));
        }
    }
    if(!line.empty()){
        ++bad;  // 末尾残留未换行的半行
        fprintf(stderr, "trailing partial line: %s\n", line.c_str());
    }
    fclose(fp);

    long expected = static_cast<long>(kThreads) * kPerThread;
    printf("expected %ld lines, got %ld, bad %ld -> %s\n",
           expected, total, bad, (total == expected && bad == 0) ? "OK" : "FAIL");
    if(total != expected || bad != 0){
        fprintf(stderr, "TestMultiThreadNoInterleave FAILED\n");
        exit(EXIT_FAILURE);
    }
}

/*
*   @brief
*   用例4：输出到指定文件
*/
static void TestFileOutput(){
    printf("--- TestFileOutput ---\n");
    FILE* fp = fopen("log.txt", "a+b");
    if(fp == nullptr){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    g_log_fp = fp;
    Logger::SetOutput([](const char* msg, size_t len){
        fwrite(msg, 1, len, g_log_fp);
    });
    Logger::SetFlush([]{ fflush(g_log_fp); });

    LOG_INFO << "custom output test";
    fflush(fp);
    fclose(fp);
    g_log_fp = nullptr;     // 必须置空，防止后续日志写入已关闭的 FILE*
    printf("log.txt written -> OK\n");
}

int main(){
    Logger::SetLogLevel(LogLevel::kDebug);
    LOG_DEBUG << "this is a debug";
    LOG_INFO << "this is an info";
    LOG_WARN << "this is an warn";
    LOG_ERROR << "this is an error";

    TestLevelFilter();
    TestCustomOutput();
    TestMultiThreadNoInterleave();
    TestFileOutput();

    Logger::SetOutput(nullptr);
    Logger::SetFlush(nullptr);
    if(g_log_fp)
        // fclose会先刷新缓冲区，再关闭流
        if(fclose(g_log_fp) != 0){  // 正常返回0表示成功：用户态缓冲区已刷到内核，文件描述符已关闭。
            perror("fclose");
        }
        g_log_fp = nullptr; // 此处关闭是防止悬空指针，因为g_log_fp指向的流已经关闭
    printf("=== ALL TESTS PASSED ===\n");
    return 0;
}
