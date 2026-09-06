CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -I. -I./src -pthread

BUILD_DIR = build

SERVER_SRCS = server.cpp \
              src/Server.cpp \
              src/Acceptor.cpp \
              src/EventLoop.cpp \
              src/Channel.cpp \
              src/Epoll.cpp \
              src/Socket.cpp \
              src/InetAddress.cpp \
              src/utils.cpp \
              src/Connection.cpp \
              src/Buffer.cpp\
              src/ThreadPool.cpp

TEST_SRCS = test.cpp \
              src/Socket.cpp \
              src/InetAddress.cpp \
              src/utils.cpp \
              src/Buffer.cpp \
              src/ThreadPool.cpp \
              src/Channel.cpp   \
              src/EventLoop.cpp \
              src/Epoll.cpp

SERVER_TARGET = $(BUILD_DIR)/server
TEST_TARGET = $(BUILD_DIR)/test

all: $(SERVER_TARGET) $(TEST_TARGET)

# $@的意思是目标名称，$^的意思是目标依赖文件列表
$(SERVER_TARGET): $(SERVER_SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^  

$(TEST_TARGET): $(TEST_SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean