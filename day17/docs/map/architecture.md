```mermaid
classDiagram
    direction LR

    class TcpServer {
        +Start()
        +HandleNewConnection(fd)
        +HandleClose(conn)
        +HandleCloseInLoop(conn)
        +SetThreadNums(n)
    }
    class EventLoopThreadPool {
        +Start()
        +next_loop() EventLoop*
        +set_thread_nums(n)
    }
    class EventLoopThread {
        +StartLoopThread() EventLoop*
        -ThreadFunc()
    }
    class Acceptor {
        +Create()
        +Bind()
        +Listen()
        +AcceptConnection()
    }
    class EventLoop {
        +Loop()
        +RunInLoop(func)
        +QueueInLoop(func)
        +DoPendingFunctors()
        +IsInLoopThread()
        +UpdateChannel(ch)
        +DeleteChannel(ch)
    }
    class Epoller {
        +Poll(timeout)
        +UpdateChannel(ch)
        +DeleteChannel(ch)
    }
    class Channel {
        +HandleEvent()
        +EnableRead()
        +EnableWrite()
        +Tie(ptr)
    }
    class TcpConnection {
        +ConnectionEstablish()
        +ConnectionDestruct()
        +HandleMessage()
        +HandleClose()
        +Send()
    }
    class Buffer

    TcpServer *-- Acceptor : acceptor_
    TcpServer *-- EventLoopThreadPool : thread_pool_
    TcpServer o-- TcpConnection : connections_map_
    TcpServer --> EventLoop : main_reactor_

    EventLoopThreadPool *-- EventLoopThread : threads_
    EventLoopThreadPool o-- EventLoop : loops_ (借用)
    EventLoopThread *-- EventLoop : 线程栈上构造 (借用)

    Acceptor --> EventLoop : loop_
    Acceptor *-- Channel : listen_channel_

    EventLoop *-- Epoller : poller_
    EventLoop *-- Channel : wakeup_channel_

    Channel --> EventLoop : loop_
    TcpConnection --> EventLoop : loop_
    TcpConnection *-- Channel : conn_channel_
    TcpConnection *-- Buffer : read/write_buffer_
```
