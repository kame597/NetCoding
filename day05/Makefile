# 编译器和编译选项
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g

# server 和 client 共用的源文件
COMMON = Socket.cpp InetAddress.cpp utils.cpp

# 默认目标：编译出 server 和 client 两个可执行文件
all: server client

# server 额外依赖 Epoll.cpp 和 Channel.cpp
server: server.cpp Epoll.cpp Channel.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) -o server server.cpp Epoll.cpp Channel.cpp $(COMMON)

# client
client: client.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) -o client client.cpp $(COMMON)

# 清理生成的可执行文件
clean:
	rm -f server client

.PHONY: all clean
