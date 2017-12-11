cmake_minimum_required(VERSION 3.8)
project(release)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        reads.cpp reads.h readsd.cpp readsd.h readsrq.cpp readsrq.h
        inBuf.h outBuf.h meminfo.cpp meminfo.h
        test.cpp timer.h)

add_executable(READS ${SOURCE_FILES})
target_compile_options(READS PRIVATE -O3 -w)
target_include_directories(READS PRIVATE ../../../sparsehash-c11)