cmake_minimum_required(VERSION 3.8)
project(APS-AllInOne)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall -O3 -g -march=native)
add_definitions(-DUSE_LOG -DTIME_RESULTS)

############### Environmental variables #####################################
set(FAKE_FILES util/inBuf.h util/outBuf.h util/meminfo.h util/timer.h
        util/sample_num.h ground_truth/graph_yche.h util/stat.h reads/reads.h ground_truth/simrank.h
        ground_truth/sparse_matrix_utils.h util/file_serialization.h
        sling/graph.h sling/sling.h sling/input_output.h sling/util.h
        probesim/evaluate.h probesim/Graph.h probesim/pooling.h probesim/Random.h probesim/SimStruct.h probesim/util.h)
set(COMMON_FILES util/meminfo.cpp util/stat.cpp util/log.cpp ${FAKE_FILES})
set(GROUND_TRUTH_FILES ground_truth/simrank.cpp ground_truth/graph_yche.cpp
        util/stat.cpp ground_truth/random_pair_generator.cpp util/log.cpp)

find_package(Boost)
find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/../../dependencies/sparsepp)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/../../dependencies/include)
include_directories(${CMAKE_SOURCE_DIR}/../../dependencies/sparsehash-yche/include)
include_directories(${CMAKE_SOURCE_DIR}/../../dependencies/SFMT)
link_libraries(tcmalloc)
link_libraries(${CMAKE_THREAD_LIBS_INIT})

############### Sling ##################
set(SLING_SOURCE_FILES
        sling/sling.cpp ${CMAKE_SOURCE_DIR}/../../dependencies/SFMT/SFMT.c sling/input_output.cpp sling/util.cpp)
add_executable(sling_ss_ap ${SLING_SOURCE_FILES} ${GROUND_TRUTH_FILES} main_single_source_all_pair.cpp)
target_compile_options(sling_ss_ap PRIVATE -O3 -g -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)
#################################

#################### ProbeSim ###############
add_executable(probesim_ss_ap
        probesim/Graph.cpp main_ss_ap.cpp ${CMAKE_SOURCE_DIR}/../../dependencies/SFMT/SFMT.c ${GROUND_TRUTH_FILES})
#############################

##################### READS-D/READS-RQ ##################################
add_executable(readsd-ap ${COMMON_FILES} ${GROUND_TRUTH_FILES} reads/readsd.cpp test_readsd_ap.cpp)
target_compile_definitions(readsd-ap PRIVATE STORE_INDEX=1)

add_executable(readsrq-ap ${COMMON_FILES} ${GROUND_TRUTH_FILES} reads/readsrq.cpp test_readsrq_ap.cpp)
target_compile_definitions(readsrq-ap PRIVATE STORE_INDEX=1)
#########################################################################

############## READS Dynamic Updates ##################################
add_executable(reads-d-dynamic-exp ${COMMON_FILES} ${GROUND_TRUTH_FILES} reads/readsd.cpp dynamic_update_exp/main_dynamic_update_reads_d.cpp)
target_compile_definitions(reads-d-dynamic-exp PRIVATE STORE_INDEX=1)

add_executable(reads-rq-dynamic-exp ${COMMON_FILES} ${GROUND_TRUTH_FILES} reads/readsrq.cpp dynamic_update_exp/main_dynamic_update_reads_rq.cpp)
target_compile_definitions(reads-rq-dynamic-exp PRIVATE STORE_INDEX=1)

add_executable(reads-d-dynamic-del ${COMMON_FILES} ${GROUND_TRUTH_FILES} reads/readsd.cpp dynamic_update_exp/main_dynamic_del_reads_d.cpp)
target_compile_definitions(reads-d-dynamic-del PRIVATE STORE_INDEX=1)

add_executable(reads-rq-dynamic-del ${COMMON_FILES} ${GROUND_TRUTH_FILES} reads/readsrq.cpp dynamic_update_exp/main_dynamic_del_reads_rq.cpp)
target_compile_definitions(reads-rq-dynamic-del PRIVATE STORE_INDEX=1)
################################################################


