set(LOCAL_STATIC_GRAPH_FILES ../algorithm/reads.cpp ../algorithm/reads.h)
set(LOCAL_COMMON_FILES ../util/inBuf.h ../util/outBuf.h ../util/meminfo.cpp ../util/meminfo.h ../util/timer.h ../util/sample_num.h)

#### reads static
add_executable(reads-ssource ${LOCAL_COMMON_FILES} ${LOCAL_STATIC_GRAPH_FILES} ./test_single_source.cpp)
target_compile_options(reads-ssource PRIVATE -O3 -g -w)
target_compile_definitions(reads-ssource PRIVATE STORE_INDEX=1)
target_include_directories(reads-ssource PRIVATE ../../../sparsehash-yche/include/)

add_executable(reads-all ${LOCAL_COMMON_FILES} ${LOCAL_STATIC_GRAPH_FILES} ./test_single_source_total.cpp)
target_compile_options(reads-all PRIVATE -O3 -g -w)
target_compile_definitions(reads-all PRIVATE STORE_INDEX=1)
target_include_directories(reads-all PRIVATE ../../../sparsehash-yche/include/)

add_executable(reads-spair ${LOCAL_COMMON_FILES} ${LOCAL_STATIC_GRAPH_FILES} ./test_single_pair.cpp)
target_compile_options(reads-spair PRIVATE -O3 -g -w)
target_compile_definitions(reads-spair PRIVATE STORE_INDEX=1 SINGLE_PAIR=1)
target_include_directories(reads-spair PRIVATE ../../../sparsehash-yche/include/)

add_executable(reads-ssource-navive ${LOCAL_COMMON_FILES} ${LOCAL_STATIC_GRAPH_FILES} ./test_single_source.cpp)
target_compile_options(reads-ssource-navive PRIVATE -O3 -g -w)
target_include_directories(reads-ssource-navive PRIVATE ../../../sparsehash-yche/include/)