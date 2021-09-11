cmake_minimum_required(VERSION 3.19)
project(deadlock_mutex)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories("include", "tests")
add_executable(deadlock_mutex
        main.cpp
        include/deadlock_detecting_mutex.h
        src/deadlock_detecting_mutex.cpp
        include/thread_mutex_graph.h
        src/thread_mutex_graph.cpp
        tests/doctest.h
        tests/main_tests.cpp
        )