project(hubppr)
# Specify the minimum version for CMake
cmake_minimum_required(VERSION 2.8)

#SET(CMAKE_CXX_FLAGS "-std=c++11 -O3 -Wall")
SET(CMAKE_CXX_FLAGS "-std=c++11 -O3 -g -w -pthread -lboost_serialization")
#SET(CMAKE_CXX_FLAGS "-std=c++11 -O3 -w -pthread")
set(CMAKE_VERBOSE_MAKEFILE ON)


find_package(Boost REQUIRED  unit_test_framework filesystem system date_time program_options serialization)
#find_package(Threads)

#set(HEADER algo.h build_oracle.h config.h graph.h head.h heap.h mylib.h query.h)
#set(PPRLIB  mylib.cpp config.cpp)

# Set the output folder where your program will be created
#set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# The following folder will be included
include_directories("${PROJECT_SOURCE_DIR}")



add_executable(hubppr ${PROJECT_SOURCE_DIR}/hubppr.cpp ${PROJECT_SOURCE_DIR}/mylib.cpp ${PROJECT_SOURCE_DIR}/config.cpp)

target_link_libraries(hubppr pthread boost_serialization)
