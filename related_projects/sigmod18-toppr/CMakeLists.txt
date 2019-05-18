cmake_minimum_required(VERSION 3.6)
project(sigmod18_toppr)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(SFMT-src-1.4.1)

add_compile_options(-march=native -ffast-math -use_fast_math -DSFMT_MEXP=607 -O3)
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(Threads REQUIRED)

add_executable(topppr
        main.cpp
        SFMT.c)
target_link_libraries(topppr PRIVATE ${CMAKE_THREAD_LIBS_INIT})
