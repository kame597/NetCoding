#include "utils.h"
#include "logger.h"

#include <cstring>  // strerror
#include <cerrno>   // errno

/*
*   @brief
*   条件成立时输出 FATAL 日志并终止进程
*   注：原实现为 perror + exit(EXIT_FAILURE)，现改为 LOG_FATAL。
*       区别在于 LOG_FATAL 会先拼出带时间戳/tid/文件行号的完整日志，
*       再 fflush 后 abort()。abort 不执行栈展开，也不调用 atexit 注册的函数，
*       因此这里必须先把 errno 的可读信息拼进消息，否则会丢失 perror 原有的诊断能力。
*       保留 errno 的读取必须在任何可能改写 errno 的调用之前完成。
*/
void errif(bool condition, const char *errmsg){
    if(condition){
        int saved_errno = errno;    // 保存最新的错误码
        LOG_FATAL << errmsg << ": " << strerror(saved_errno);   // 将错误码翻译成字符串
    }
}
