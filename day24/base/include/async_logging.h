#pragma once
#include "fixed_buffer.h"
#include "common.h"
#include "latch.h"

#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <cstdio>
#include <condition_variable>
#include <string>

class AsyncLogging{
public:
    DISALLOW_COPY_AND_MOVE(AsyncLogging);
    using Buffer = FixedBuffer<kLargeBuffer>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferVector = std::vector<BufferPtr>;
    AsyncLogging(const std::string& basename, int flush_interval);
    ~AsyncLogging();

    void Start();   // 启动后端线程
    void Stop();    // 关闭后端线程并排空缓冲区
    void Append(const char* msg, size_t len);   // 供前端传入

private:
    void ThreadFunc();
    std::mutex mutex_;
    std::condition_variable cond_;
    std::thread thread_;
    Latch latch_;       // Latch用于保证后端线程已创建，因为只有一个后端线程，所以latch值设为1

    std::atomic<bool> running_;
    int flush_interval_;    // 前后端缓冲区交换的时间间隔
    
    std::string basename_;
    FILE* fp_;

    BufferVector buffers_;
    BufferPtr curr_buffer_;
    BufferPtr next_buffer_;
};