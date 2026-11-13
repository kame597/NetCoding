#include "event_loop.h"
#include "http_server.h"
#include "logger.h"
#include <memory>
#include <iostream>
int main(int argc, char* args[]){

    Logger::SetLogFile("log.txt");
    Logger::SetInterval(3);
    Logger::SetLogLevel(LogLevel::kError);

    int port = 8888;
    std::string ip = "127.0.0.1";
    std::unique_ptr<EventLoop> loop = std::make_unique<EventLoop>();
    std::unique_ptr<HttpServer> server = std::make_unique<HttpServer>(loop.get(), ip.c_str(), port);

    // 压测用：预生成不同体积的固定正文，避免把构造成本算进业务开销
    static const std::string kBody1K(1024, 'a');
    static const std::string kBody16K(16 * 1024, 'b');
    static const std::string kBody64K(64 * 1024, 'c');

    // 具体业务注入
    server->set_message_callback([](const HttpRequest& req, HttpResponse* resp){
        resp->set_version("HTTP/1.1");
        const std::string& url = req.url();
        if(url == "/"){
            resp->set_status_code(HttpResponse::StatusCode::k200Ok);
            resp->add_header("Content-Type", "text/plain");
            resp->set_body("this is a test\n");
        }else if(url == "/1k"){
            resp->set_status_code(HttpResponse::StatusCode::k200Ok);
            resp->add_header("Content-Type", "text/plain");
            resp->set_body(kBody1K);
        }else if(url == "/16k"){
            resp->set_status_code(HttpResponse::StatusCode::k200Ok);
            resp->add_header("Content-Type", "text/plain");
            resp->set_body(kBody16K);
        }else if(url == "/64k"){
            resp->set_status_code(HttpResponse::StatusCode::k200Ok);
            resp->add_header("Content-Type", "text/plain");
            resp->set_body(kBody64K);
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