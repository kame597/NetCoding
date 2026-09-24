cmake_minimum_required(VERSION 3.10)
project(WebServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS "-g -Wall")

find_package(Threads REQUIRED)

aux_source_directory(base/src SRC_LIST1)
aux_source_directory(tcp/src SRC_LIST2)
aux_source_directory(http/src SRC_LIST3)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

#把库代码编译成一个名为webServer的静态库
add_library(webServer STATIC ${SRC_LIST1} ${SRC_LIST2} ${SRC_LIST3})

#指定编译时的头文件搜索路径 -I
target_include_directories(webServer PUBLIC base/include tcp/include http/include)

#指定链接时挂的库
target_link_libraries(webServer PUBLIC Threads::Threads)

add_executable(echoserver test/echo_server.cpp)
target_link_libraries(echoserver PRIVATE webServer)

add_executable(client test/test.cpp)
target_link_libraries(client PRIVATE webServer)

add_executable(http_test test/http_test.cpp)
target_link_libraries(http_test PRIVATE webServer)