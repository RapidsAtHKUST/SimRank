cmake_minimum_required(VERSION 3.8)
project(APS-AllInOne)
SET(CMAKE_BUILD_TYPE "Release")
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
        util/random_utils.h
        dynamic_algorithms/local_push.h
        ground_truth/simrank.h
        reads/inBuf.h reads/outBuf.h util/meminfo.h util/timer.h
        reads/sample_num.h util/graph_yche.h util/stat.h reads/reads.h
        util/search_yche.h
        util/sparse_matrix_utils.h util/file_serialization.h
        sling/graph.h sling/sling.h sling/input_output.h sling/util.h
        probesim/evaluate.h probesim/Graph.h probesim/pooling.h probesim/Random.h probesim/SimStruct.h probesim/util.h
        local_push/parallel_local_push_yche_with_lock.h
        local_push/parallel_local_push_yche.h local_push/local_push_yche.h
        yche_refactor/bprw_yche.h yche_refactor/local_push_yche.h
        yche_refactor/flpmc_yche.h yche_refactor/parallel_local_push/parallel_local_push_yche.h
        yche_refactor/bflpmc_yche.h
        pcg_tkde17/tkde17_sim.h)

set(COMMON_UTIL_FILES
        ${CMAKE_SOURCE_DIR}/../../dependencies/SFMT/SFMT.c
        ground_truth/simrank.cpp ground_truth/random_pair_generator.cpp
        util/graph_yche.cpp util/graph_ywangby.cpp util/search_yche.cpp
        util/random_pair_generator.cpp
        util/stat.cpp util/log.cpp util/meminfo.cpp ${FAKE_FILES})
set(OUR_COMMON_LIB commons-lib)
add_library(${OUR_COMMON_LIB} ${COMMON_UTIL_FILES})
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

################################# LIN ###################
add_executable(LinSimBench lineard14/main_lineard_ap.cpp lineard14/linearD.cpp)
################################# MCAP #########################
add_executable(CloudWalkerBench cloud_walker15/cloud_walker.cpp cloud_walker15/main_cloud_ap.cpp)
#############################  PCG  #############################
add_executable(pcg pcg_tkde17/main_pcg.cpp pcg_tkde17/tkde17_sim.cpp)


##################### Dynamic-Graph Algorithms ###########
################### dynamic-flp
add_executable(dynamic-flp dynamic_algorithms/main_dynamic_exp.cpp dynamic_algorithms/local_push.cpp)
target_compile_definitions(dynamic-flp PRIVATE -DFLP=1)
############# dynamic-rlp
add_executable(dynamic-rlp dynamic_algorithms/main_dynamic_exp.cpp dynamic_algorithms/local_push.cpp)
###### dynamic tsf
add_executable(dynamic-tsf dynamic_algorithms/main_dynamic_tsf_exp.cpp)

############## READS Dynamic Updates ##################################
add_executable(reads-d-dynamic-exp reads/readsd.cpp dynamic_algorithms/main_dynamic_update_reads_d.cpp)
target_compile_definitions(reads-d-dynamic-exp PRIVATE STORE_INDEX=1)

add_executable(reads-rq-dynamic-exp reads/readsrq.cpp dynamic_algorithms/main_dynamic_update_reads_rq.cpp)
target_compile_definitions(reads-rq-dynamic-exp PRIVATE STORE_INDEX=1)

add_executable(reads-d-dynamic-del reads/readsd.cpp dynamic_algorithms/main_dynamic_del_reads_d.cpp)
target_compile_definitions(reads-d-dynamic-del PRIVATE STORE_INDEX=1)

add_executable(reads-rq-dynamic-del reads/readsrq.cpp dynamic_algorithms/main_dynamic_del_reads_rq.cpp)
target_compile_definitions(reads-rq-dynamic-del PRIVATE STORE_INDEX=1)
################################################################

set(BPRW_FILES yche_refactor/bprw_yche.cpp)
set(LPMC_FILES yche_refactor/flpmc_yche.cpp
        yche_refactor/local_push_yche.cpp
        yche_refactor/parallel_local_push/parallel_local_push_yche.cpp
        yche_refactor/parallel_local_push/parallel_local_push_yche_rlp.cpp)

set(BLPMC_SOURCE_FILES ${BPRW_FILES} playground/main_bprw.cpp)

############################ MC Algorithm 1: BPRW #############################################
add_executable(bprw ${BLPMC_SOURCE_FILES})
add_executable(bprw-std ${BLPMC_SOURCE_FILES})
#### bprw(all pair)
add_executable(bprw-ap ${BPRW_FILES} playground/main_bprw_all_pair.cpp)
target_compile_definitions(bprw-ap PRIVATE -DSFMT=1 -DGROUND_TRUTH=1 -DALL_PAIR=1)
#### bprw benchmark
add_executable(bprw-bench ${BPRW_FILES} playground/main_bprw_all_pair.cpp)
target_compile_definitions(bprw-bench PRIVATE -DSFMT=1)
#### bprw(random pairs)
add_executable(bprw-rand-bench ${BPRW_FILES} playground/random_pairs/main_bprw_rand_pair.cpp)
target_compile_definitions(bprw-rand-bench PRIVATE -DSFMT=1)
add_executable(bprw-rand-bench-gt ${BPRW_FILES} playground/random_pairs/main_bprw_rand_pair.cpp)
target_compile_definitions(bprw-rand-bench-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1)
#### bprw(random paris varying parameters)
add_executable(bprw-rand-varying ${BPRW_FILES}
        playground/varying_parameters/main_bprw_rand_pair_varying_parameters.cpp)
target_compile_definitions(bprw-rand-varying PRIVATE -DSFMT=1)
## add groud-truth support
add_executable(bprw-rand-varying-gt ${BPRW_FILES}
        playground/varying_parameters/main_bprw_rand_pair_varying_parameters.cpp)
target_compile_definitions(bprw-rand-varying-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1)
##############################################################################################

################################# MC Algorithm 2: FLPMC #############################################################
set(FLPMC_SOURCE_FILES ${LPMC_FILES} playground/main_flpmc.cpp)
add_executable(flpmc ${FLPMC_SOURCE_FILES})
target_compile_definitions(flpmc PRIVATE -DSFMT=1)
#### flpmc(all pair)
set(FLPMC_SOURCE_FILES ${LPMC_FILES} playground/main_flpmc_all_pair.cpp)
add_executable(flpmc-ap ${FLPMC_SOURCE_FILES})
target_compile_definitions(flpmc-ap PRIVATE -DSFMT=1 -DGROUND_TRUTH=1 -DALL_PAIR=1)
#### flpmc(benchmark)
add_executable(flpmc-bench ${FLPMC_SOURCE_FILES})
target_compile_definitions(flpmc-bench PRIVATE -DSFMT=1)
#### flpmc(random pairs)
set(FLPMC_SOURCE_FILES_RAND ${LPMC_FILES} playground/random_pairs/main_flpmc_rand_pair.cpp)
add_executable(flpmc-rand-bench ${FLPMC_SOURCE_FILES_RAND})
target_compile_definitions(flpmc-rand-bench PRIVATE -DSFMT=1)
add_executable(flpmc-rand-bench-gt ${FLPMC_SOURCE_FILES_RAND})
target_compile_definitions(flpmc-rand-bench-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1)
#### flpmc(random pairs, varying r_max)
set(FLPMC_RMAX_SOURCE_FILES_RAND ${LPMC_FILES}
        playground/varying_rmax/main_flpmc_rand_pair_varying_rmax.cpp)
add_executable(flpmc-rand-rmax ${FLPMC_RMAX_SOURCE_FILES_RAND})
target_compile_definitions(flpmc-rand-rmax PRIVATE -DSFMT=1 -DVARYING_RMAX=1)
add_executable(flpmc-rand-rmax-gt ${FLPMC_RMAX_SOURCE_FILES_RAND})
target_compile_definitions(flpmc-rand-rmax-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1 -DVARYING_RMAX=1)
#### flpmc(random pairs varying parameters)
set(FLPMC_SOURCE_FILES_RAND ${LPMC_FILES}
        playground/varying_parameters/main_flpmc_rand_pair_varying_parameters.cpp)
add_executable(flpmc-rand-varying ${FLPMC_SOURCE_FILES_RAND})
target_compile_definitions(flpmc-rand-varying PRIVATE -DSFMT=1)
add_executable(flpmc-rand-varying-gt ${FLPMC_SOURCE_FILES_RAND})
target_compile_definitions(flpmc-rand-varying-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1)

##############################  MC Algorithm 3: BFLPMC ##############################################3333
set(BFLPMC_SOURCE_FILES ${LPMC_FILES} ${BPRW_FILES}
        yche_refactor/bflpmc_yche.cpp playground/main_bflpmc.cpp)
add_executable(bflpmc ${BFLPMC_SOURCE_FILES})
target_compile_definitions(bflpmc PRIVATE -DSFMT=1)

#### bflpmc(all pair)
set(BFLPMC_SOURCE_FILES ${LPMC_FILES} ${BPRW_FILES} yche_refactor/bflpmc_yche.cpp playground/main_bflpmc_all_pair.cpp)
add_executable(bflpmc-ap ${BFLPMC_SOURCE_FILES})
target_compile_definitions(bflpmc-ap PRIVATE -DSFMT=1 -DGROUND_TRUTH=1 -DGROUND_TRUTH_STATISTICS=1 -DALL_PAIR=1)

#### bflpmc(benchmark)
add_executable(bflpmc-bench ${BFLPMC_SOURCE_FILES})
target_compile_definitions(bflpmc-bench PRIVATE -DSFMT=1)
#### bflpmc(random pairs)
set(BFLPMC_SOURCE_FILES_RAND ${LPMC_FILES} ${BPRW_FILES}
        yche_refactor/bflpmc_yche.cpp playground/random_pairs/main_bflpmc_rand_pair.cpp)
add_executable(bflpmc-rand-bench ${BFLPMC_SOURCE_FILES_RAND})
target_compile_definitions(bflpmc-rand-bench PRIVATE -DSFMT=1)
add_executable(bflpmc-rand-bench-gt ${BFLPMC_SOURCE_FILES_RAND})
target_compile_definitions(bflpmc-rand-bench-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1 -DGROUND_TRUTH_STATISTICS=1)
#### bflpmc(random pairs, varying r_max)
set(BFLPMC_RMAX_SOURCE_FILES_RAND ${LPMC_FILES} ${BPRW_FILES} yche_refactor/bflpmc_yche.cpp
        playground/varying_rmax/main_bflpmc_rand_pair_varying_rmax.cpp)
add_executable(bflpmc-rand-rmax ${BFLPMC_RMAX_SOURCE_FILES_RAND})
target_compile_definitions(bflpmc-rand-rmax PRIVATE -DSFMT=1 -DVARYING_RMAX=1)
add_executable(bflpmc-rand-rmax-gt ${BFLPMC_RMAX_SOURCE_FILES_RAND})
target_compile_definitions(bflpmc-rand-rmax-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1 -DVARYING_RMAX=1)
#### bflpmc(random pairs varying parameters)
set(BFLPMC_SOURCE_FILES_RAND ${LPMC_FILES} ${BPRW_FILES} yche_refactor/bflpmc_yche.cpp
        playground/varying_parameters/main_bflpmc_rand_pair_varying_parameters.cpp)
add_executable(bflpmc-rand-varying ${BFLPMC_SOURCE_FILES_RAND})
target_compile_definitions(bflpmc-rand-varying PRIVATE -DSFMT=1)
add_executable(bflpmc-rand-varying-gt ${BFLPMC_SOURCE_FILES_RAND})
target_compile_definitions(bflpmc-rand-varying-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1)

####################### reduced local push testing
add_executable(rlp-test playground/reduced_local_push/main_exec_reduced_local_push.cpp ${LPMC_FILES})
target_compile_definitions(rlp-test PRIVATE -DSFMT=1)