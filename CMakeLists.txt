cmake_minimum_required(VERSION 3.10)
project(Thread)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    test/iomanager_test.cc
    src/Thread.cc
    src/Fiber.cc
    src/Scheduler.cc
    src/Timer.cc
    src/IOManager.cc
)

add_executable(iomanager_test ${SOURCES})

target_include_directories(iomanager_test PRIVATE include)

find_package(Threads REQUIRED)
target_link_libraries(iomanager_test PRIVATE Threads::Threads)
