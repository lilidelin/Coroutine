cmake_minimum_required(VERSION 3.10)
project(Thread)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    test/scheduler_test.cc
    src/Thread.cc
    src/Fiber.cc
    src/Scheduler.cc
)

add_executable(scheduler_test ${SOURCES})

target_include_directories(scheduler_test PRIVATE include)

find_package(Threads REQUIRED)
target_link_libraries(scheduler_test PRIVATE Threads::Threads)
