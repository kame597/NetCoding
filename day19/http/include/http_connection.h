#pragma once
#include <memory>
#include "tcp_connection.h"
#include "common.h"
#include "http_context.h"


//HttpConnection是包裹TcpConnection，是一种组合关系
class HttpConnection{
public:
    DISALLOW_COPY_AND_MOVE(HttpConnection);
    explicit HttpConnection(const std::shared_ptr<TcpConnection>& conn);
    ~HttpConnection();

    //访问底层的tcp连接
    const std::shared_ptr<TcpConnection>& tcp_conn() const;

    // 获取解析状态
    // todo 为什么不用常引用呢？
    HttpContext& context();

    int keepalive_count() const;
    void incr_keepalive();

    bool should_close() const; 
    void set_should_close(bool);

private:
    std::shared_ptr<TcpConnection> conn_;   //不用裸指针是防止野指针，因为TcpConnection被shared_map管理，有可能被误析构
    HttpContext context_;   //用于解析报文
    int keepalive_count_ = 0;       // 计数器，计数这个连接已经处理了多少请求，用于长连接
    bool should_close_ = false;     // 标记位，表示这个连接处理完当前请求后，应该被关闭
};

/*
*   依赖倒置本意是指，高层模块不应该依赖低层模块，两者都应该依赖抽象；
*   抽象不应该依赖细节，细节应该依赖抽象。
*   即：不稳定的模块，应该依赖稳定的模块；比如具体实现依赖抽象接口
*/ 