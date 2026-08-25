CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -I./src

# 输出目录
BUILD_DIR = build

# server 依赖的全部源文件
SERVER_SRCS = server.cpp \
              src/Server.cpp \
              src/Acceptor.cpp \
              src/EventLoop.cpp \
              src/Channel.cpp \
              src/Epoll.cpp \
              src/Socket.cpp \
              src/InetAddress.cpp \
              src/utils.cpp

# client 依赖的源文件
CLIENT_SRCS = client.cpp \
              src/Socket.cpp \
              src/InetAddress.cpp \
              src/utils.cpp

# 最终生成的可执行文件路径
SERVER_TARGET = $(BUILD_DIR)/server
CLIENT_TARGET = $(BUILD_DIR)/client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SERVER_SRCS)

$(CLIENT_TARGET): $(CLIENT_SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(CLIENT_SRCS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
