#pragma once

#include <stdint.h>
#include <pthread.h>
#include <stdio.h>


/*
*   声明一个命名空间，把一对相关的工具函数和全局变量聚合在一起
*   避免了项目中的命名污染
*/
namespace current_thread{ 

    /*
    *   __thread是GCC的线程局部存储关键字
    *   被__thread修饰的变量，每个线程都会有一份独立拷贝，互不影响
    *   即：不同线程有各自独立的值，不会共享
    */
    extern __thread int t_cachedTid;
    extern __thread char t_formattedTid[32];
    extern __thread int t_formattedTidLength;

    void CacheTid();

    pid_t get_tid();

    inline int tid(){
        /*
        *   __builtin_expect(表达式，期望值)
        *   GCC的分支预测提示，意为(t_cachedTid == 0)这个条件大概率为false(0)
        *   其本质上就是 t_cachedTid == 0，第二个参数只是给编译器的提示，本质上就是条件判断
        * 
        *   因此其含义就是：如果t_cachedTid == 0, 那么就调用CacheTid获取当前线程tid
        */
        if(__builtin_expect(t_cachedTid == 0, 0)) 
            CacheTid();
        return t_cachedTid;
    }

    inline const char* tid_string(){return t_formattedTid; }
    inline int tid_string_length(){return t_formattedTidLength; }

}   //namespace current_thread