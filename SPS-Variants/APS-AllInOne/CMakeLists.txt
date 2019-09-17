cmake_minimum_required(VERSION 3.8)
project(APS-AllInOne)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall -O3 -g -march=native)
add_definitions(-DUSE_LOG -DTIME_RESULTS -DHAS_OPENMP)

if (NOT DATA_ROOT_PATH)
    message("not given")
endif ()
add_compile_definitions(DATA_ROOT="${DATA_ROOT_PATH}")
message("Data Root Dir: ${DATA_ROOT_PATH}")
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH}) # To allow CMake to locate our Find*.cmake files

############### Environmental variables #####################################
set(FAKE_FILES util/inBuf.h util/outBuf.h util/meminfo.h util/timer.h
        util/sample_num.h util/graph_yche.h util/stat.h reads/reads.h ground_truth/simrank.h
        util/sparse_matrix_utils.h util/file_serialization.h
        sling/graph.h sling/sling.h sling/input_output.h sling/util.h
        probesim/evaluate.h probesim/Graph.h probesim/pooling.h probesim/Random.h probesim/SimStruct.h probesim/util.h
        local_push/parallel_local_push_yche_with_lock.h
        local_push/parallel_local_push_yche.h local_push/local_push_yche.h)
set(COMMON_FILES util/meminfo.cpp util/stat.cpp util/log.cpp ${FAKE_FILES})
set(GROUND_TRUTH_FILES ground_truth/simrank.cpp util/graph_yche.cpp
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

################################## FLP/RLP/PRLP/PFLP
set(UTIL_FILES util/stat.cpp util/log.cpp)
set(PLP_FILES local_push/parallel_local_push_yche_with_lock.cpp local_push/parallel_local_push_yche_rlp_with_lock.cpp)
set(PLP_LOCK_FREE_FILES local_push/parallel_local_push_yche.cpp local_push/parallel_local_push_yche_rlp_lock_free.cpp)
set(LP_FILES local_push/local_push_yche.cpp local_push/local_push_yche_rlp.cpp ${PLP_FILES})

add_subdirectory(playground)
add_executable(prlp-lock-free-stat playground/main_exec_parallel_lp.cpp ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_LOCK_FREE_FILES})
target_compile_definitions(prlp-lock-free-stat PRIVATE -DPUSH_NUM_STAT=1)

add_executable(rlp playground/main_exec_reduced_local_push.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${LP_FILES})
add_executable(flp playground/main_exec_reduced_local_push.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${LP_FILES})
target_compile_definitions(flp PRIVATE -DFLP=1)

########## lock-based FLP/RLP
add_executable(pflp-with-lock playground/main_exec_parallel_lp_with_lock.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_FILES})
target_compile_definitions(pflp-with-lock PRIVATE -DFLP=1)

add_executable(prlp-with-lock playground/main_exec_parallel_lp_with_lock.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_FILES})
#target_compile_definitions(prlp-with-lock PRIVATE -DDEBUG=1)

add_executable(prlp-with-lock-stat playground/main_exec_parallel_lp_with_lock.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_FILES})
target_compile_definitions(prlp-with-lock-stat PRIVATE -DPUSH_NUM_STAT=1)

######## lock-free FLP(not-supported)/RLP
add_executable(pflp-lock-free playground/main_exec_parallel_lp.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_LOCK_FREE_FILES})
target_compile_definitions(pflp-lock-free PRIVATE -DFLP=1)

add_executable(prlp-lock-free playground/main_exec_parallel_lp.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_LOCK_FREE_FILES})
target_compile_definitions(prlp-lock-free PRIVATE -DDEBUG=1)

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


