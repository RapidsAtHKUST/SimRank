cmake_minimum_required(VERSION 3.6)
project(LPMC-Optimizing)

################################ 1st: basic environmental variables ######################################
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

################################ 2nd: libarary environmental variables ######################################
find_package(Boost REQUIRED program_options)
if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
endif ()
find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

################################ 3rd: source file environmental variables ######################################
set(UTIL_FILES util/file_serialization.h util/stat.cpp util/stat.h
        util/sfmt_based_rand.h ../SFMT/SFMT.c util/graph_yche.cpp)
set(BIN_SEARCH_FIES util/search_yche.h util/search_yche.cpp)
set(GROUND_TRUTH_FILES yche_refactor/simrank.cpp yche_refactor/simrank.h)
set(SFMT_COMPILE_FLAGS -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)

set(BPRW_FILES yche_refactor/bprw_yche.cpp yche_refactor/bprw_yche.h ${BIN_SEARCH_FIES})
set(LPMC_FILES yche_refactor/flpmc_yche.h yche_refactor/flpmc_yche.cpp
        yche_refactor/local_push_yche.h yche_refactor/local_push_yche.cpp util/sparse_matrix_utils.h
        yche_refactor/parallel_local_push/parallel_local_push_yche.h yche_refactor/parallel_local_push/parallel_local_push_yche.cpp
        yche_refactor/parallel_local_push/parallel_local_push_yche_rlp.cpp)

############################## general macros ###################
macro(InitBoostHashSFMTEigen exec_name)
    target_include_directories(${exec_name} PRIVATE ${Boost_INCLUDE_DIRS})
    target_include_directories(${exec_name} PRIVATE ../sparsehash-yche/include)
    target_include_directories(${exec_name} PRIVATE ../sparsepp)
    target_include_directories(${exec_name} PRIVATE ../SFMT)
    target_include_directories(${exec_name} PRIVATE ../include)
    target_compile_definitions(${exec_name} PRIVATE -DHAS_OPENMP=1)
endmacro()

#### sub cmake dir: playground  ######################################
add_subdirectory(playground)
# add_subdirectory(util)

#### 1st: bprw #######################################################################################################
macro(InitBprw exec_name)
    InitBoostHashSFMTEigen(${exec_name})
    target_link_libraries(${exec_name} PRIVATE tcmalloc)
    target_compile_options(${exec_name} PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS} -march=native)
#    target_compile_options(${exec_name} PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
endmacro()

set(BLPMC_SOURCE_FILES ${BPRW_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} playground/main_bprw.cpp)
add_executable(bprw ${BLPMC_SOURCE_FILES})
InitBprw(bprw)
target_compile_definitions(bprw PRIVATE -DSFMT=1)
#target_compile_definitions(bprw PRIVATE -DSFMT=1 -DDEBUG=1)

#### bprw(std rand gen)
add_executable(bprw-std ${BLPMC_SOURCE_FILES})
InitBprw(bprw-std)
#target_compile_definitions(bprw-std PRIVATE -DDEBUG=1)

#### bprw(all pair)
add_executable(bprw-ap ${BPRW_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} playground/main_bprw_all_pair.cpp)
InitBprw(bprw-ap)
target_compile_definitions(bprw-ap PRIVATE -DSFMT=1 -DGROUND_TRUTH=1 -DALL_PAIR=1)

#### bprw benchmark
add_executable(bprw-bench ${BPRW_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} playground/main_bprw_all_pair.cpp)
InitBprw(bprw-bench)
target_compile_definitions(bprw-bench PRIVATE -DSFMT=1)
#target_compile_definitions(bprw-bench PRIVATE -DSFMT=1 -DSPARSE_HASH_MAP_FOR_HEAP=1)

#### bprw(random pairs)
add_executable(bprw-rand-bench ${BPRW_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} playground/random_pairs/main_bprw_rand_pair.cpp
        util/random_pair_generator.cpp)
InitBprw(bprw-rand-bench)
target_compile_definitions(bprw-rand-bench PRIVATE -DSFMT=1)

add_executable(bprw-rand-bench-gt ${BPRW_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} playground/random_pairs/main_bprw_rand_pair.cpp
        util/random_pair_generator.cpp)
InitBprw(bprw-rand-bench-gt)
target_compile_definitions(bprw-rand-bench-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1)

#### bprw(random paris varying parameters)
add_executable(bprw-rand-varying ${BPRW_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES}
        playground/varying_parameters/main_bprw_rand_pair_varying_parameters.cpp
        util/random_pair_generator.cpp)
InitBprw(bprw-rand-varying)
target_compile_definitions(bprw-rand-varying PRIVATE -DSFMT=1)

## add groud-truth support
add_executable(bprw-rand-varying-gt ${BPRW_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES}
        playground/varying_parameters/main_bprw_rand_pair_varying_parameters.cpp
        util/random_pair_generator.cpp)
InitBprw(bprw-rand-varying-gt)
target_compile_definitions(bprw-rand-varying-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1)

#### 2nd: flpmc #######################################################################################################
macro(InitFLPMC exec_name)
    InitBoostHashSFMTEigen(${exec_name})
    message(${Boost_LIBRARIES})
    target_link_libraries(${exec_name} PRIVATE ${Boost_LIBRARIES})
    target_link_libraries(${exec_name} PRIVATE tcmalloc)
    target_compile_options(${exec_name} PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS} -march=native)
endmacro()
set(FLPMC_SOURCE_FILES ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} playground/main_flpmc.cpp)

add_executable(flpmc ${FLPMC_SOURCE_FILES})
InitFLPMC(flpmc)
target_compile_definitions(flpmc PRIVATE -DSFMT=1)

#### flpmc(all pair)
set(FLPMC_SOURCE_FILES ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} playground/main_flpmc_all_pair.cpp)
add_executable(flpmc-ap ${FLPMC_SOURCE_FILES})
InitFLPMC(flpmc-ap)
target_compile_definitions(flpmc-ap PRIVATE -DSFMT=1 -DGROUND_TRUTH=1 -DALL_PAIR=1)

#### flpmc(benchmark)
add_executable(flpmc-bench ${FLPMC_SOURCE_FILES})
InitFLPMC(flpmc-bench)
target_compile_definitions(flpmc-bench PRIVATE -DSFMT=1)

#### flpmc(random pairs)
set(FLPMC_SOURCE_FILES_RAND ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} playground/random_pairs/main_flpmc_rand_pair.cpp
        util/random_pair_generator.cpp)
add_executable(flpmc-rand-bench ${FLPMC_SOURCE_FILES_RAND})
InitFLPMC(flpmc-rand-bench)
target_compile_definitions(flpmc-rand-bench PRIVATE -DSFMT=1)

add_executable(flpmc-rand-bench-gt ${FLPMC_SOURCE_FILES_RAND})
InitFLPMC(flpmc-rand-bench-gt)
target_compile_definitions(flpmc-rand-bench-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1)

#### flpmc(random pairs, varying r_max)
set(FLPMC_RMAX_SOURCE_FILES_RAND ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES}
        playground/varying_rmax/main_flpmc_rand_pair_varying_rmax.cpp
        util/random_pair_generator.cpp)
add_executable(flpmc-rand-rmax ${FLPMC_RMAX_SOURCE_FILES_RAND})
InitFLPMC(flpmc-rand-rmax)
target_compile_definitions(flpmc-rand-rmax PRIVATE -DSFMT=1 -DVARYING_RMAX=1)

add_executable(flpmc-rand-rmax-gt ${FLPMC_RMAX_SOURCE_FILES_RAND})
InitFLPMC(flpmc-rand-rmax-gt)
target_compile_definitions(flpmc-rand-rmax-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1 -DVARYING_RMAX=1)

#### flpmc(random pairs varying parameters)
set(FLPMC_SOURCE_FILES_RAND ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES}
        playground/varying_parameters/main_flpmc_rand_pair_varying_parameters.cpp
        util/random_pair_generator.cpp)
add_executable(flpmc-rand-varying ${FLPMC_SOURCE_FILES_RAND})
InitFLPMC(flpmc-rand-varying)
target_compile_definitions(flpmc-rand-varying PRIVATE -DSFMT=1)

set(FLPMC_SOURCE_FILES_RAND ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES}
        playground/topk/main_flpmc_topk.cpp
        util/random_pair_generator.cpp)
add_executable(flpmc-topk ${FLPMC_SOURCE_FILES_RAND})
InitFLPMC(flpmc-topk)
target_compile_definitions(flpmc-topk PRIVATE -DSFMT=1)
#target_compile_definitions(flpmc-topk PRIVATE -DVARYING_RMAX=1)

add_executable(flpmc-rand-varying-gt ${FLPMC_SOURCE_FILES_RAND})
InitFLPMC(flpmc-rand-varying-gt)
target_compile_definitions(flpmc-rand-varying-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1)

#### 3rd: bflpmc #######################################################################################################
macro(InitBFLPMC exec_name)
    InitFLPMC(${exec_name})
endmacro()
set(BFLPMC_SOURCE_FILES ${LPMC_FILES} ${BPRW_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES}
        yche_refactor/bflpmc_yche.h yche_refactor/bflpmc_yche.cpp playground/main_bflpmc.cpp)
add_executable(bflpmc ${BFLPMC_SOURCE_FILES})
InitBFLPMC(bflpmc)
target_compile_definitions(bflpmc PRIVATE -DSFMT=1)

#### bflpmc(all pair)
set(BFLPMC_SOURCE_FILES ${LPMC_FILES} ${BPRW_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES}
        yche_refactor/bflpmc_yche.h yche_refactor/bflpmc_yche.cpp playground/main_bflpmc_all_pair.cpp)
add_executable(bflpmc-ap ${BFLPMC_SOURCE_FILES})
InitBFLPMC(bflpmc-ap)
target_compile_definitions(bflpmc-ap PRIVATE -DSFMT=1 -DGROUND_TRUTH=1 -DGROUND_TRUTH_STATISTICS=1 -DALL_PAIR=1)

#### bflpmc(benchmark)
add_executable(bflpmc-bench ${BFLPMC_SOURCE_FILES})
InitBFLPMC(bflpmc-bench)
target_compile_definitions(bflpmc-bench PRIVATE -DSFMT=1)
#target_compile_definitions(bflpmc-bench PRIVATE -DSFMT=1 -DSPARSE_HASH_MAP_FOR_HEAP=1)

#### bflpmc(random pairs)
set(BFLPMC_SOURCE_FILES_RAND ${LPMC_FILES} ${BPRW_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES}
        yche_refactor/bflpmc_yche.h yche_refactor/bflpmc_yche.cpp playground/random_pairs/main_bflpmc_rand_pair.cpp
        util/random_pair_generator.cpp)
add_executable(bflpmc-rand-bench ${BFLPMC_SOURCE_FILES_RAND})
InitBFLPMC(bflpmc-rand-bench)
target_compile_definitions(bflpmc-rand-bench PRIVATE -DSFMT=1)

add_executable(bflpmc-rand-bench-gt ${BFLPMC_SOURCE_FILES_RAND})
InitBFLPMC(bflpmc-rand-bench-gt)
target_compile_definitions(bflpmc-rand-bench-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1 -DGROUND_TRUTH_STATISTICS=1)

#### bflpmc(random pairs, varying r_max)
set(BFLPMC_RMAX_SOURCE_FILES_RAND ${LPMC_FILES} ${BPRW_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES}
        yche_refactor/bflpmc_yche.h yche_refactor/bflpmc_yche.cpp
        playground/varying_rmax/main_bflpmc_rand_pair_varying_rmax.cpp
        util/random_pair_generator.cpp)

add_executable(bflpmc-rand-rmax ${BFLPMC_RMAX_SOURCE_FILES_RAND})
InitBFLPMC(bflpmc-rand-rmax)
target_compile_definitions(bflpmc-rand-rmax PRIVATE -DSFMT=1 -DVARYING_RMAX=1)

add_executable(bflpmc-rand-rmax-gt ${BFLPMC_RMAX_SOURCE_FILES_RAND})
InitBFLPMC(bflpmc-rand-rmax-gt)
target_compile_definitions(bflpmc-rand-rmax-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1 -DVARYING_RMAX=1)

#### bflpmc(random pairs varying parameters)
set(BFLPMC_SOURCE_FILES_RAND ${LPMC_FILES} ${BPRW_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES}
        yche_refactor/bflpmc_yche.h yche_refactor/bflpmc_yche.cpp playground/varying_parameters/main_bflpmc_rand_pair_varying_parameters.cpp
        util/random_pair_generator.cpp)
add_executable(bflpmc-rand-varying ${BFLPMC_SOURCE_FILES_RAND})
InitBFLPMC(bflpmc-rand-varying)
target_compile_definitions(bflpmc-rand-varying PRIVATE -DSFMT=1)

add_executable(bflpmc-rand-varying-gt ${BFLPMC_SOURCE_FILES_RAND})
InitBFLPMC(bflpmc-rand-varying-gt)
target_compile_definitions(bflpmc-rand-varying-gt PRIVATE -DSFMT=1 -DGROUND_TRUTH=1)

####################### reduced local push testing
add_executable(rlp playground/reduced_local_push/main_exec_reduced_local_push.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES})
InitFLPMC(rlp)
target_compile_definitions(rlp PRIVATE -DSFMT=1)
