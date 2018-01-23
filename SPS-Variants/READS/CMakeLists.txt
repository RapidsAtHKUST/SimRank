cmake_minimum_required(VERSION 3.8)
project(release)

set(CMAKE_CXX_STANDARD 11)


set(COMMON_FILES util/inBuf.h util/outBuf.h util/meminfo.cpp util/meminfo.h util/timer.h util/sample_num.h)
set(STATIC_GRAPH_FILES algorithm/reads.cpp algorithm/reads.h)

#### reads static
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

#### reads release by the original authors
add_executable(reads-original-release ${COMMON_FILES} algorithm/readsd.h algorithm/readsd.cpp ${STATIC_GRAPH_FILES} algorithm/readsrq.h algorithm/readsrq.cpp test.cpp)
target_compile_options(reads-original-release PRIVATE -O3 -g -w)
target_include_directories(reads-original-release PRIVATE ../../sparsehash-yche/include/)