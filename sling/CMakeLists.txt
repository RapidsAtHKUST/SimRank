cmake_minimum_required(VERSION 3.6)
project(sling)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        graph.h
        sling.h
        main.cpp)

add_executable(sling ${SOURCE_FILES})