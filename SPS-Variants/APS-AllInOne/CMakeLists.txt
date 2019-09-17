cmake_minimum_required(VERSION 3.8)
project(APS-AllInOne)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall -O3 -g -march=native)
add_definitions(-DUSE_LOG -DTIME_RESULTS -DHAS_OPENMP -DSFMT_MEXP=607 -DHAVE_SSE2)

if (NOT DATA_ROOT_PATH)
    message("not given")
endif ()
add_compile_definitions(DATA_ROOT="${DATA_ROOT_PATH}")
message("Data Root Dir: ${DATA_ROOT_PATH}")
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH}) # To allow CMake to locate our Find*.cmake files

############### Environmental variables #####################################
include(InitLibraries)

###### For Jetbrains Indexing Files #############################
set(FAKE_FILES
        ground_truth/simrank.h
        reads/inBuf.h reads/outBuf.h util/meminfo.h util/timer.h
        reads/sample_num.h util/graph_yche.h util/stat.h reads/reads.h
        util/sparse_matrix_utils.h util/file_serialization.h
        sling/graph.h sling/sling.h sling/input_output.h sling/util.h
        probesim/evaluate.h probesim/Graph.h probesim/pooling.h probesim/Random.h probesim/SimStruct.h probesim/util.h
        local_push/parallel_local_push_yche_with_lock.h
        local_push/parallel_local_push_yche.h local_push/local_push_yche.h)

set(GROUND_TRUTH_FILES
        ${CMAKE_SOURCE_DIR}/../../dependencies/SFMT/SFMT.c
        ground_truth/simrank.cpp ground_truth/random_pair_generator.cpp
        util/graph_yche.cpp util/graph_ywangby.cpp
        util/stat.cpp util/log.cpp util/meminfo.cpp ${FAKE_FILES})
set(OUR_COMMON_LIB commons-lib)
add_library(${OUR_COMMON_LIB} ${GROUND_TRUTH_FILES})
link_libraries(${OUR_COMMON_LIB})

################################## FLP/RLP ##################################
set(PLP_FILES local_push/parallel_local_push_yche_with_lock.cpp local_push/parallel_local_push_yche_rlp_with_lock.cpp)
set(PLP_LOCK_FREE_FILES local_push/parallel_local_push_yche.cpp local_push/parallel_local_push_yche_rlp_lock_free.cpp)
set(LP_FILES local_push/local_push_yche.cpp local_push/local_push_yche_rlp.cpp ${PLP_FILES})

add_subdirectory(playground)

add_executable(rlp local_push/main_exec_reduced_local_push.cpp ${LP_FILES})
add_executable(flp local_push/main_exec_reduced_local_push.cpp ${LP_FILES})
target_compile_definitions(flp PRIVATE -DFLP=1)

########## lock-based FLP/RLP ##################################
add_executable(pflp-with-lock local_push/main_exec_parallel_lp_with_lock.cpp ${PLP_FILES})
target_compile_definitions(pflp-with-lock PRIVATE -DFLP=1)

add_executable(prlp-with-lock local_push/main_exec_parallel_lp_with_lock.cpp ${PLP_FILES})

add_executable(prlp-with-lock-stat local_push/main_exec_parallel_lp_with_lock.cpp ${PLP_FILES})
target_compile_definitions(prlp-with-lock-stat PRIVATE -DPUSH_NUM_STAT=1)

######## lock-free FLP(not-supported)/RLP ##################################
add_executable(pflp-lock-free local_push/main_exec_parallel_lp.cpp ${PLP_LOCK_FREE_FILES})
target_compile_definitions(pflp-lock-free PRIVATE -DFLP=1)

add_executable(prlp-lock-free local_push/main_exec_parallel_lp.cpp ${PLP_LOCK_FREE_FILES})
target_compile_definitions(prlp-lock-free PRIVATE -DDEBUG=1)

add_executable(prlp-lock-free-stat local_push/main_exec_parallel_lp.cpp ${PLP_LOCK_FREE_FILES})
target_compile_definitions(prlp-lock-free-stat PRIVATE -DPUSH_NUM_STAT=1)
####################################################################################3

############### Sling ####################################################
set(SLING_SOURCE_FILES
        sling/sling.cpp sling/input_output.cpp sling/util.cpp)
add_executable(sling_ss_ap ${SLING_SOURCE_FILES} sling/main_single_source_all_pair.cpp)
###################################################################

#################### ProbeSim #################################################
add_executable(probesim_ss_ap probesim/Graph.cpp probesim/main_ss_ap.cpp)
###############################################################

##################### READS-D/READS-RQ ##################################
add_executable(readsd-ap reads/readsd.cpp reads/test_readsd_ap.cpp)
target_compile_definitions(readsd-ap PRIVATE STORE_INDEX=1)

add_executable(readsrq-ap reads/readsrq.cpp reads/test_readsrq_ap.cpp)
target_compile_definitions(readsrq-ap PRIVATE STORE_INDEX=1)
#########################################################################

############## READS Dynamic Updates ##################################
add_executable(reads-d-dynamic-exp reads/readsd.cpp reads/dynamic_update_exp/main_dynamic_update_reads_d.cpp)
target_compile_definitions(reads-d-dynamic-exp PRIVATE STORE_INDEX=1)

add_executable(reads-rq-dynamic-exp reads/readsrq.cpp reads/dynamic_update_exp/main_dynamic_update_reads_rq.cpp)
target_compile_definitions(reads-rq-dynamic-exp PRIVATE STORE_INDEX=1)

add_executable(reads-d-dynamic-del reads/readsd.cpp reads/dynamic_update_exp/main_dynamic_del_reads_d.cpp)
target_compile_definitions(reads-d-dynamic-del PRIVATE STORE_INDEX=1)

add_executable(reads-rq-dynamic-del reads/readsrq.cpp reads/dynamic_update_exp/main_dynamic_del_reads_rq.cpp)
target_compile_definitions(reads-rq-dynamic-del PRIVATE STORE_INDEX=1)
################################################################

add_executable(LinSimBench lineard14/main_lineard_ap.cpp lineard14/linearD.cpp)
add_executable(CloudWalkerBench cloud_walker15/cloud_walker.cpp cloud_walker15/main_cloud_ap.cpp)

