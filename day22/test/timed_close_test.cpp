#include "event_loop.h"
#include "http_server.h"
#include <memory>
#include <iostream>

int main(){
    std::unique_ptr<EventLoop> loop = std::make_unique<EventLoop>();
    std::unique_ptr<HttpServer> server = std::make_unique<HttpServer>(loop.get(), "127.0.0.1", 8888);
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
    server->SetConnTimeout(10);
    server->Start();
}