cmake_minimum_required(VERSION 3.8)
project(release)

set(CMAKE_CXX_STANDARD 11)


set(COMMON_FILES inBuf.h outBuf.h meminfo.cpp meminfo.h timer.h)
set(STATIC_GRAPH_FILES reads.cpp reads.h)

add_executable(reads-ssource ${COMMON_FILES} ${STATIC_GRAPH_FILES} test_single_source.cpp)
target_compile_options(reads-ssource PRIVATE -O3 -g -w)
target_compile_definitions(reads-ssource PRIVATE STORE_INDEX=1)
target_include_directories(reads-ssource PRIVATE ../../sparsehash-yche/include/)

add_executable(reads-all ${COMMON_FILES} ${STATIC_GRAPH_FILES} test_single_source_total.cpp)
target_compile_options(reads-all PRIVATE -O3 -g -w)
target_compile_definitions(reads-all PRIVATE STORE_INDEX=1)
target_include_directories(reads-all PRIVATE ../../sparsehash-yche/include/)

add_executable(reads-spair ${COMMON_FILES} ${STATIC_GRAPH_FILES} test_single_pair.cpp)
target_compile_options(reads-spair PRIVATE -O3 -g -w)
target_compile_definitions(reads-spair PRIVATE STORE_INDEX=1 SINGLE_PAIR=1)
target_include_directories(reads-spair PRIVATE ../../sparsehash-yche/include/)

add_executable(reads-ssource-navive ${COMMON_FILES} ${STATIC_GRAPH_FILES} test_single_source.cpp)
target_compile_options(reads-ssource-navive PRIVATE -O3 -g -w)
target_include_directories(reads-ssource-navive PRIVATE ../../sparsehash-yche/include/)

