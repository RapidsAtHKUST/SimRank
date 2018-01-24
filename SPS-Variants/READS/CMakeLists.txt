cmake_minimum_required(VERSION 3.8)
project(release)

set(CMAKE_CXX_STANDARD 11)

# environmental variables
set(COMMON_FILES util/inBuf.h util/outBuf.h util/meminfo.cpp util/meminfo.h util/timer.h util/sample_num.h)
set(STATIC_GRAPH_FILES algorithm/reads.cpp algorithm/reads.h)
set(GROUND_TRUTH_FILES ground_truth/simrank.cpp ground_truth/simrank.h ground_truth/graph_yche.cpp ground_truth/graph_yche.h
        ground_truth/sparse_matrix_utils.h ground_truth/file_serialization.h ground_truth/stat.cpp ground_truth/stat.h)

find_package(Boost)
find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

macro(InitREADS exec_name)
    target_include_directories(${exec_name} PRIVATE ../../sparsepp)
    target_include_directories(${exec_name} PRIVATE ${Boost_INCLUDE_DIRS})
    target_include_directories(${exec_name} PRIVATE ../../include)
    target_include_directories(${exec_name} PRIVATE ../../sparsehash-yche/include)

    target_compile_options(${exec_name} PRIVATE -O3 -g)
    target_link_libraries(${exec_name} tcmalloc)
endmacro()

#### reads static
add_executable(reads-ssource ${COMMON_FILES} ${STATIC_GRAPH_FILES} static_solution/test_single_source.cpp)
target_compile_options(reads-ssource PRIVATE -O3 -g -w)
target_compile_definitions(reads-ssource PRIVATE STORE_INDEX=1)
target_include_directories(reads-ssource PRIVATE ../../sparsehash-yche/include/)

add_executable(reads-all ${COMMON_FILES} ${STATIC_GRAPH_FILES} static_solution/test_single_source_total.cpp)
target_compile_options(reads-all PRIVATE -O3 -g -w)
target_compile_definitions(reads-all PRIVATE STORE_INDEX=1)
target_include_directories(reads-all PRIVATE ../../sparsehash-yche/include/)

add_executable(reads-spair ${COMMON_FILES} ${STATIC_GRAPH_FILES} static_solution/test_single_pair.cpp)
target_compile_options(reads-spair PRIVATE -O3 -g -w)
target_compile_definitions(reads-spair PRIVATE STORE_INDEX=1 SINGLE_PAIR=1)
target_include_directories(reads-spair PRIVATE ../../sparsehash-yche/include/)

add_executable(reads-ssource-navive ${COMMON_FILES} ${STATIC_GRAPH_FILES} static_solution/test_single_source.cpp)
target_compile_options(reads-ssource-navive PRIVATE -O3 -g -w)
target_include_directories(reads-ssource-navive PRIVATE ../../sparsehash-yche/include/)

#### reads release by the original authors
add_executable(reads-original-release ${COMMON_FILES} algorithm/readsd.h algorithm/readsd.cpp ${STATIC_GRAPH_FILES} algorithm/readsrq.h algorithm/readsrq.cpp test.cpp)
target_compile_options(reads-original-release PRIVATE -O3 -g -w)
target_include_directories(reads-original-release PRIVATE ../../sparsehash-yche/include/)

############### 1st: reads static single-pair
add_executable(reads-s ${COMMON_FILES} ${GROUND_TRUTH_FILES} algorithm/reads.cpp test_reads_sp.cpp)
InitREADS(reads-s)
target_compile_definitions(reads-s PRIVATE STORE_INDEX=1)

############### 2nd: reads dynamic single-pair
add_executable(reads-d ${COMMON_FILES} ${GROUND_TRUTH_FILES} algorithm/readsd.cpp test_readsd_sp.cpp)
InitREADS(reads-d)
target_compile_definitions(reads-d PRIVATE STORE_INDEX=1)

############### 3rd: reads-rq single-pair
add_executable(reads-rq ${COMMON_FILES} ${GROUND_TRUTH_FILES} algorithm/readsrq.cpp test_reads_rq_sp.cpp)
InitREADS(reads-rq)
target_compile_definitions(reads-rq PRIVATE STORE_INDEX=1)