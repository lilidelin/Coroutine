cmake_minimum_required(VERSION 3.10)
project(Thread)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    test/timer_test.cc
    src/Thread.cc
    src/Fiber.cc
    src/Scheduler.cc
    src/Timer.cc
)

add_executable(timer_test ${SOURCES})

target_include_directories(timer_test PRIVATE include)

find_package(Threads REQUIRED)
target_link_libraries(timer_test PRIVATE Threads::Threads)
