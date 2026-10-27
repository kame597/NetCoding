#include "event_loop.h"
#include "http_server.h"
#include <memory>
#include <iostream>
int main(int argc, char* args[]){
    int port = 8888;
    std::string ip = "127.0.0.1";
    std::unique_ptr<EventLoop> loop = std::make_unique<EventLoop>();
    std::unique_ptr<HttpServer> server = std::make_unique<HttpServer>(loop.get(), ip.c_str(), port);

    // 具体业务注入
    server->set_message_callback([](const HttpRequest& req, HttpResponse* resp){
        std::cout << "url = [" << req.url() << "]" << std::endl;
        resp->set_version("HTTP/1.1");
        if(req.url() == "/"){
            resp->set_status_code(HttpResponse::StatusCode::k200Ok);
            resp->add_header("Content-Type", "text/plain");
            resp->set_body("this is a test\n");
        }else{
            resp->set_status_code(HttpResponse::StatusCode::k404NotFound);
            resp->set_body("404 Not Found\n");
        }
    });
    server->Start();
    return 0;
}

/*
*   HTTP/1.1 200 OK\r\n			   状态行
*   Content-Encoding: gzip\r\n	   响应头	
*   Content-Type: text/html\r\n
*   Content-Length: 5\r\n
*   \r\n						       空行
*   hello						       响应正文
*
*   status_code_ = 200
*   version_ = HTTP/1.1
*   header_ = {"Content-Encoding" -> "gzip", "Content-Type" -> "text", ...}
*   body_ = "hello"
*/