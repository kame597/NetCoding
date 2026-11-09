#include "async_logging.h"
#include <assert.h>
#include <chrono>


AsyncLogging::AsyncLogging(const std::string& basename, int flush_interval) : 
    latch_(1),
    running_(false),
    flush_interval_(flush_interval),
    basename_(basename),
    fp_(nullptr),
    curr_buffer_(std::make_unique<Buffer>()),
    next_buffer_(std::make_unique<Buffer>())
{
    curr_buffer_->Bzero();
    next_buffer_->Bzero();

    if(basename_.empty()){
        fp_ = stdout;
    }
    else{
        fp_ = fopen(basename_.c_str(), "w+b");  // 先绑定文件流，再开启后端线程
        if(fp_ == nullptr){
            fp_ = stdout;
            printf("日志输出文件 %s 打开失败", basename_.c_str());
        }
    }
    Start();
}

AsyncLogging::~AsyncLogging()
{
    Stop();
    if(fp_ != nullptr && fp_ != stdout){
        fclose(fp_);
        fp_ = nullptr;
    }
}

void AsyncLogging::Start()
{
    running_ = true;
    thread_ = std::thread(&AsyncLogging::ThreadFunc, this);
    latch_.Wait();  // 等待后端CountDown
}

void AsyncLogging::Stop()
{
    {
        std::lock_guard lock(mutex_);   // 因为Start开启了后端线程，因此产生数据竞争
        running_ = false;
    }
    cond_.notify_one();     // 唤醒后端线程，告知其running_已更新
    if(thread_.joinable()) thread_.join();
}

/*
*   @brief
*   接受前端数据，存入缓冲，当缓冲已满，塞入就绪缓冲队列
*/
void AsyncLogging::Append(const char *msg, size_t len)
{
    if(len == 0) return; 
    std::lock_guard lock(mutex_);    // 其保护curr_buffer_, buffers_, next_buffer_三个状态

    if(curr_buffer_ -> AvailBytes() >= len){    
        curr_buffer_->Append(msg, len);
    }
    else{   // 缓冲已满，放入后端缓冲队列
        buffers_.push_back(std::move(curr_buffer_));
        if(next_buffer_)    // 如果有备用缓冲，则直接换成备用缓冲
            curr_buffer_ = std::move(next_buffer_);
        else{    // 没备用缓冲，则重新分配
            curr_buffer_ = std::make_unique<Buffer>();
            curr_buffer_ -> Bzero();
        }

        curr_buffer_->Append(msg, len); //保证这一轮的日志不会丢
        cond_.notify_one(); //  通知消费线程开写
    }
    
}

void AsyncLogging::ThreadFunc()
{
    assert(running_ == true);
    latch_.CountDown();
    BufferPtr buffer1 = std::make_unique<Buffer>();
    BufferPtr buffer2 = std::make_unique<Buffer>();
    buffer1->Bzero();
    buffer2->Bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);

    while(running_){
        {
            std::unique_lock lock(mutex_);
            if(buffers_.empty()){   //如果就绪缓冲队列为空，则用默认刷新时间
                cond_.wait_for(lock, std::chrono::seconds(flush_interval_));
            }
            buffers_.push_back(std::move(curr_buffer_));    // 当前缓冲塞入缓冲队列 
            curr_buffer_ = std::move(buffer1);              // 分配新缓冲给前端

            buffersToWrite.swap(buffers_);  // 前后端交换

            if(!next_buffer_)   // todo 优化： 是否可以在需要时再创建buffer2给next_buffer_，节省一定内存分配开销
                next_buffer_ = std::move(buffer2);  // 若前端没有备用缓冲，则分配备用缓冲
        }
        
        assert(!buffersToWrite.empty());    // 交换后buffersToWrite不应为空

        for(const auto& buffer : buffersToWrite){
            if(buffer->Length() > 0)
                fwrite(buffer->data(), 1, buffer->Length(), fp_);
        }
        fflush(fp_);

        // 回收缓冲对象，这一步是必要的，因为buffer1和buffer2分给前端后，自身就是nullptr，需要重新分配
        if(buffersToWrite.size() > 2){
            buffersToWrite.resize(2);
        }
        if(!buffer1 && !buffersToWrite.empty()){
            buffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            buffer1->Reset();
        }
        if(!buffer2 && !buffersToWrite.empty()){
            buffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            buffer2->Reset();
        }
        buffersToWrite.clear();
    }

    // 退出前排空，防止主线程Stop后，后端写完第一个缓冲后判定while退出，导致存在buffer未处理
    {
        std::lock_guard lock(mutex_);
        if(curr_buffer_ && curr_buffer_->Length() > 0){
            buffers_.push_back(std::move(curr_buffer_));
        }
        buffersToWrite.swap(buffers_);
    }
    for(const auto& buffer : buffersToWrite){
        if(buffer->Length() > 0){
            fwrite(buffer->data(), 1, buffer->Length(), fp_);
        }
    }
    fflush(fp_);

}
