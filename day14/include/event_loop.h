#pragma once
#include "common.h"
#include <memory>
#include <functional>
class Epoller;
class Channel;

class EventLoop{
public:
    DISALLOW_COPY_AND_MOVE(EventLoop);
    EventLoop();
    ~EventLoop();

    void Loop() const;
    void UpdateChannel(Channel*) const;
    void DeleteChannel(Channel*) const;
private:
    std::unique_ptr<Epoller> poller_;
};