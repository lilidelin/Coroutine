cmake_minimum_required(VERSION 3.10)
project(Thread)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    test/fiber_test.cc
    src/Thread.cc
    src/Fiber.cc
)

add_executable(fiber_test ${SOURCES})

target_include_directories(fiber_test PRIVATE include)

find_package(Threads REQUIRED)
target_link_libraries(fiber_test PRIVATE Threads::Threads)
