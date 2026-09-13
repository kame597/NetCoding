CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -O2 -I. -I./include -pthread

BUILD_DIR = build

SERVER_SRCS = echo_server.cpp \
              src/acceptor.cc \
              src/buffer.cc \
              src/channel.cc \
              src/epoller.cc \
              src/event_loop.cc \
              src/tcp_connection.cpp \
              src/tcp_server.cpp \
              src/thread_pool.cpp \
              src/utils.cpp \
              src/current_thread.cc

CLIENT_SRCS = test.cpp \
              src/thread_pool.cpp

SERVER_TARGET = $(BUILD_DIR)/server
CLIENT_TARGET = $(BUILD_DIR)/client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

# $@ 是目标名，$^ 是所有依赖文件列表
$(SERVER_TARGET): $(SERVER_SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(CLIENT_TARGET): $(CLIENT_SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
