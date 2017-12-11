cmake_minimum_required(VERSION 3.6)
project(sling)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES graph.h sling.h main.cpp ../../SFMT/SFMT.c)

# setup pthread environmental variables
find_package(Threads REQUIRED)

add_executable(sling ${SOURCE_FILES})
target_include_directories(sling PRIVATE ../../sparsehash-yche/include/)
target_include_directories(sling PRIVATE ../../SFMT)
target_link_libraries(sling PRIVATE ${CMAKE_THREAD_LIBS_INIT})

target_compile_options(sling PRIVATE -O3 -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2 )
