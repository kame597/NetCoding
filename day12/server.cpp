#include "src/EventLoop.h"
#include "src/Server.h"
#include <memory>

int main(){
    std::unique_ptr<EventLoop> loop = std::make_unique<EventLoop>();
    std::unique_ptr<Server> server = std::make_unique<Server>(loop.get());
    loop->loop();
    return 0;
}