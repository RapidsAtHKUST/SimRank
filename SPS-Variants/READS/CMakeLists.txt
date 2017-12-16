cmake_minimum_required(VERSION 3.8)
project(release)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        inBuf.h outBuf.h meminfo.cpp meminfo.h timer.h
        reads.cpp reads.h
        readsd.cpp readsd.h
        readsrq.cpp readsrq.h
        test.cpp)

add_executable(READS ${SOURCE_FILES})
target_compile_options(READS PRIVATE -O3 -g -w)
target_compile_definitions(READS PRIVATE STORE_INDEX=1)
target_include_directories(READS PRIVATE ../../sparsehash-yche/include/)

add_executable(READS_NAIVE ${SOURCE_FILES})
target_compile_options(READS_NAIVE PRIVATE -O3 -g -w)
target_include_directories(READS_NAIVE PRIVATE ../../sparsehash-yche/include/)

