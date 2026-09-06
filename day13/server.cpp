#include "src/EventLoop.h"
#include "src/Server.h"
#include "src/Socket.h"
#include "src/Buffer.h"
#include "src/Connection.h"
#include <memory>
#include <mutex>
#include <iostream>

int main(){
    std::unique_ptr<EventLoop> loop = std::make_unique<EventLoop>();
    std::unique_ptr<Server> server = std::make_unique<Server>(loop.get());
    

    /*
    *   @brief
    *   注入具体的业务逻辑.
    *   最终会绑定到Connection的on_connect_callback回调函数上
    *   最终由Channel执行
    */
    server->OnConnect([](Connection* conn){   //Connection指针，代表服务器到客户端的连接
        conn->Read();
        if(conn->GetState() == Connection::State::Closed){
            conn->Close();
            return;
        }
        
        std::cout << "从客户端" << conn->GetSocket()->getFd() << "收到信息:"
                << conn->ReadBuffer() << std::endl;
        
        conn->SetWriteBuffer(conn->ReadBuffer());
        conn->Write();  
    });
    
    loop->loop();
    return 0;
}