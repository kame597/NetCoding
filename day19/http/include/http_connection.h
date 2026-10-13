#pragma once
#include <memory>

#include "common.h"
#include "buffer.h"
#include "tcp_connection.h"
#include "http_context.h"
#include "http_response.h"
#include "http_request.h"


//HttpConnection是包裹TcpConnection，是一种组合关系
/*
*   HttpConnection驱动HttpContext来负责报文的解析与响应报文的构造
*   响应报文的具体构造方法，由HttpServer外部确定
*/
class HttpConnection{
public:
    using MessageCallback = std::function<void(const HttpRequest&, HttpResponse*)>;
    DISALLOW_COPY_AND_MOVE(HttpConnection);
    explicit HttpConnection(const std::shared_ptr<TcpConnection>& conn);
    ~HttpConnection();

    // 对外动作接口
    void OnMessage(Buffer* buf);    
    void OnClose();

    // 回调注入接口
    void set_message_callback(MessageCallback cb);

    // 只读访问接口
    const std::shared_ptr<TcpConnection>& tcp_conn() const;
    int keepalive_count() const;
    bool should_close() const; 

private:
    // 内部实现方法
    void HandleRequest(const HttpRequest& req);     //解析请求报文
    void SendResponse(const HttpResponse& resp);    //发送响应报文
    void Reset();

    MessageCallback message_callback_;
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

/*
*   回调的精髓是控制反转：
*   上层负责策略（决定做什么），下层负责行为（决定何时触发）。
*   控制流在注册时从上往下，事件与请求数据在触发时从下往上，响应数据则从上往下写回。
*/

/*
*   控制反转IoC的含义是指，把调用的控制权（调用什么，什么时候调用），从下层反转交给上层控制
*   正向依赖：下层直接依赖上层，例如Tcp依赖Http，会导致修改业务要连同Tcp一起改，添加新的协议也极其不方便
*   反转依赖： 下层依赖抽象，上层注入策略。比如TcpConnection只依赖on_message_这个抽象，具体调用由上层注册决定
*   依赖倒置是一种原则，控制反转是依赖倒置的效果，回调是实现的手段
*/

/*
*   单一职责原则SRC，HttpServer只负责连接管理，HttpConnection负责协议流程控制，HttpContext负责报文解析
*/