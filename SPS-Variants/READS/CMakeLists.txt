cmake_minimum_required(VERSION 3.8)
project(release)

set(CMAKE_CXX_STANDARD 11)

# environmental variables
set(COMMON_FILES util/inBuf.h util/outBuf.h util/meminfo.cpp util/meminfo.h util/timer.h util/sample_num.h
        ground_truth/stat.cpp)
set(STATIC_GRAPH_FILES algorithm/reads.cpp algorithm/reads.h)
set(GROUND_TRUTH_FILES ground_truth/simrank.cpp ground_truth/simrank.h ground_truth/graph_yche.cpp ground_truth/graph_yche.h
        ground_truth/sparse_matrix_utils.h ground_truth/file_serialization.h ground_truth/stat.cpp ground_truth/stat.h
        ground_truth/random_pair_generator.cpp)

find_package(Boost)
find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

macro(InitREADS exec_name)
    target_include_directories(${exec_name} PRIVATE ../../dependencies/sparsepp)
    target_include_directories(${exec_name} PRIVATE ${Boost_INCLUDE_DIRS})
    target_include_directories(${exec_name} PRIVATE ../../dependencies/include)
    target_include_directories(${exec_name} PRIVATE ../../dependencies/sparsehash-yche/include)

    target_compile_options(${exec_name} PRIVATE -O3 -g)
    target_link_libraries(${exec_name} tcmalloc)
endmacro()

add_subdirectory(static_solution)

#### reads release by the original authors
add_executable(reads-original-release ${COMMON_FILES} ${STATIC_GRAPH_FILES}
        algorithm/readsd.h algorithm/readsd.cpp algorithm/readsrq.h algorithm/readsrq.cpp test.cpp)
target_compile_options(reads-original-release PRIVATE -O3 -g -w)
target_include_directories(reads-original-release PRIVATE ../../dependencies/sparsehash-yche/include/)

############### 1st: reads static single-pair
add_executable(reads-s ${COMMON_FILES} ${GROUND_TRUTH_FILES} algorithm/reads.cpp test_reads_sp.cpp)
InitREADS(reads-s)
target_compile_definitions(reads-s PRIVATE STORE_INDEX=1)

#### reads-bench (random pairs)
add_executable(reads-s-rand-bench ${COMMON_FILES} ${GROUND_TRUTH_FILES} algorithm/reads.cpp rand_pair_exec/test_reads_rand_pair.cpp)
InitREADS(reads-s-rand-bench)
target_compile_definitions(reads-s-rand-bench PRIVATE STORE_INDEX=1)

############### 2nd: reads dynamic single-pair
add_executable(reads-d ${COMMON_FILES} ${GROUND_TRUTH_FILES} algorithm/readsd.cpp test_readsd_sp.cpp)
InitREADS(reads-d)
target_compile_definitions(reads-d PRIVATE STORE_INDEX=1)

#### reads-d (random pairs)
add_executable(reads-d-rand-bench ${COMMON_FILES} ${GROUND_TRUTH_FILES} algorithm/readsd.cpp rand_pair_exec/test_readsd_rand_pair.cpp)
InitREADS(reads-d-rand-bench)
target_compile_definitions(reads-d-rand-bench PRIVATE STORE_INDEX=1)

############### 3rd: reads-rq single-pair
add_executable(reads-rq ${COMMON_FILES} ${GROUND_TRUTH_FILES} algorithm/readsrq.cpp test_reads_rq_sp.cpp)
InitREADS(reads-rq)
target_compile_definitions(reads-rq PRIVATE STORE_INDEX=1)

#### reads-rq (random pairs)
add_executable(reads-rq-rand-bench ${COMMON_FILES} ${GROUND_TRUTH_FILES} algorithm/readsrq.cpp rand_pair_exec/test_reads_rq_rand_pair.cpp)
InitREADS(reads-rq-rand-bench)
target_compile_definitions(reads-rq-rand-bench PRIVATE STORE_INDEX=1)

############## 4th: dynamic experiments
add_executable(reads-d-dynamic-exp ${COMMON_FILES} ${GROUND_TRUTH_FILES} algorithm/readsd.cpp dynamic_update_exp/main_dynamic_update_reads_d.cpp)
InitREADS(reads-d-dynamic-exp)
target_compile_definitions(reads-d-dynamic-exp PRIVATE STORE_INDEX=1)

add_executable(reads-rq-dynamic-exp ${COMMON_FILES} ${GROUND_TRUTH_FILES} algorithm/readsrq.cpp dynamic_update_exp/main_dynamic_update_reads_rq.cpp)
InitREADS(reads-rq-dynamic-exp)
target_compile_definitions(reads-rq-dynamic-exp PRIVATE STORE_INDEX=1)

add_executable(reads-d-dynamic-del ${COMMON_FILES} ${GROUND_TRUTH_FILES} algorithm/readsd.cpp dynamic_update_exp/main_dynamic_del_reads_d.cpp)
InitREADS(reads-d-dynamic-del)
target_compile_definitions(reads-d-dynamic-del PRIVATE STORE_INDEX=1)

add_executable(reads-rq-dynamic-del ${COMMON_FILES} ${GROUND_TRUTH_FILES} algorithm/readsrq.cpp dynamic_update_exp/main_dynamic_del_reads_rq.cpp)
InitREADS(reads-rq-dynamic-del)
target_compile_definitions(reads-rq-dynamic-del PRIVATE STORE_INDEX=1)

