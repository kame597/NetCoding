CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -I. -I./src -pthread

BUILD_DIR = build
TEST_DIR = test

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

CLIENT_SRCS = client.cpp \
              src/Socket.cpp \
              src/InetAddress.cpp \
              src/utils.cpp \
              src/Buffer.cpp

THREADPOOLTEST_SRCS = test/ThreadPoolTest.cpp \
                      src/ThreadPool.cpp

SERVER_TARGET = $(BUILD_DIR)/server
CLIENT_TARGET = $(BUILD_DIR)/client
THREADPOOLTEST_TARGET = $(TEST_DIR)/threadpooltest

all: $(SERVER_TARGET) $(CLIENT_TARGET) $(THREADPOOLTEST_TARGET)

# $@的意思是目标名称，$^的意思是目标依赖文件列表
$(SERVER_TARGET): $(SERVER_SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^  

$(CLIENT_TARGET): $(CLIENT_SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(THREADPOOLTEST_TARGET): $(THREADPOOLTEST_SRCS)
	@mkdir -p $(TEST_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf $(BUILD_DIR) $(TEST_DIR)/threadpooltest

.PHONY: all clean