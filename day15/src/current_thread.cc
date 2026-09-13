#include "include/current_thread.h"
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>

namespace current_thread{

    __thread int t_cachedTid = 0;
    __thread char t_formattedTid[32];
    __thread int t_formattedTidLength;

    void CacheTid(){
        if(t_cachedTid == 0){
            t_cachedTid = get_tid();
            /*
            *   snprintf：
            *   把t_cachedTid格式化为字符串，存进t_formattedTid中
            *   %5d 是右对齐，宽度为5的整数，用于打印日志
            *   返回值为实际写入的字符数，不包含结尾的\0，存入t_formattedTidLength
            */
            t_formattedTidLength = snprintf(t_formattedTid, sizeof(t_formattedTid), "%5d ", t_cachedTid);
        }
    }

    pid_t get_tid(){
        /*
        *   syscall(SYS_gettid)的作用：
        *   直接发起系统调用，获取内核线程id
        */
        return static_cast<int>(syscall(SYS_gettid));
    }
}