cmake_minimum_required(VERSION 3.10)
project(Thread)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Thread_test test/Thread_test.cc src/Thread.cc)

target_include_directories(Thread_test PRIVATE include)

find_package(Threads REQUIRED)
target_link_libraries(Thread_test PRIVATE Threads::Threads)
