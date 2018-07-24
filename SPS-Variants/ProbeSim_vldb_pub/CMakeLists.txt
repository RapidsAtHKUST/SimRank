cmake_minimum_required(VERSION 3.10)
project(ProbeSim_vldb_pub)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

find_package(Threads)

add_executable(ProbeSim
        evaluate.h Graph.h pooling.h Random.h SimStruct.h util.h main.cpp
        ../../SFMT/SFMT.c)
target_compile_options(ProbeSim PRIVATE -O3 -march=core2 -DSFMT_MEXP=607)
target_include_directories(ProbeSim PRIVATE ../../SFMT/)
target_link_libraries(ProbeSim PRIVATE ${CMAKE_THREAD_LIBS_INIT})