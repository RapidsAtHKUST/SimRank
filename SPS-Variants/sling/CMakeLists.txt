cmake_minimum_required(VERSION 3.6)
project(sling)

## To allow CMake to locate our Find*.cmake files
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 11)
find_package(TBB REQUIRED)
include_directories(${TBB_INCLUDE_DIRS})
link_libraries(${TBB_LIBRARIES})

if (NOT DATA_ROOT_PATH)
    message("not given")
    set(DATA_ROOT_PATH ".")
endif ()
add_compile_definitions(DATA_ROOT="${DATA_ROOT_PATH}")
add_compile_definitions(USE_LOG)
message("Data Root Dir: ${DATA_ROOT_PATH}")

set(FAKE_FILES ground_truth/graph_yche.h
        ground_truth/sparse_matrix_utils.h ground_truth/file_serialization.h ground_truth/stat.h
        ground_truth/yche_serialization.h ground_truth/simrank.h graph.h sling.h input_output.h util.h)
set(GROUND_TRUTH_FILES ground_truth/simrank.cpp ground_truth/graph_yche.cpp)
set(SOURCE_FILES util/log.cpp
        sling.cpp ../../dependencies/SFMT/SFMT.c input_output.cpp util.cpp ground_truth/stat.cpp)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_compile_options(-Wall -O3 -g -march=native)
add_definitions(-DUSE_LOG -DTIME_RESULTS)
include_directories(${CMAKE_SOURCE_DIR})

# setup pthread environmental variables
find_package(Boost REQUIRED program_options)
if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
endif ()
find_package(Threads REQUIRED)
find_package(OpenMP)

if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

macro(InitHashSFMTEigenPthread exec_name)
    target_include_directories(${exec_name} PRIVATE ../../dependencies/sparsehash-yche/include/)
    target_include_directories(${exec_name} PRIVATE ../../dependencies/SFMT)
    target_include_directories(${exec_name} PRIVATE ../../dependencies/include)

    target_link_libraries(${exec_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${exec_name} PRIVATE tcmalloc)

    target_compile_options(${exec_name} PRIVATE -O3 -g -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)
endmacro()

## 1st: sling single pair
#add_executable(sling ${SOURCE_FILES} main.cpp)
#InitHashSFMTEigenPthread(sling)

# 2nd: sling all pair
macro(InitSlingAP exec_name)
    target_include_directories(${exec_name} PRIVATE ../../dependencies/sparsepp)
    InitHashSFMTEigenPthread(${exec_name})
endmacro()

add_executable(sling_ss ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_ss.cpp)
InitSlingAP(sling_ss)
target_compile_definitions(sling_ss PRIVATE -DALWAYS_REBUILD=1)

#add_executable(sling_ss_ap ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_single_source_all_pair.cpp)
#InitSlingAP(sling_ss_ap)
#
#add_executable(sling_all ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_query_all_vertices.cpp)
#InitSlingAP(sling_all)
#target_compile_definitions(sling_all PRIVATE -DGROUND_TRUTH=1 -DALL_PAIR=1)
##target_compile_definitions(sling_all PRIVATE -DGROUND_TRUTH=1)
#
########### benchmark
#add_executable(sling_bench ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_query_all_vertices.cpp)
#InitSlingAP(sling_bench)
#
##### sling (random pairs)
#add_executable(sling-rand-bench ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_query_rand_pairs.cpp ground_truth/random_pair_generator.cpp)
#InitSlingAP(sling-rand-bench)
#
#add_executable(sling-rand-bench-gt ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_query_rand_pairs.cpp ground_truth/random_pair_generator.cpp)
#InitSlingAP(sling-rand-bench-gt)
#target_compile_definitions(sling-rand-bench-gt PRIVATE -DGROUND_TRUTH=1)
#
##### sling (random pairs, varying parameters)
#add_executable(sling-rand-varying ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_query_rand_pairs_varying_parameters.cpp ground_truth/random_pair_generator.cpp)
#InitSlingAP(sling-rand-varying)
#target_compile_definitions(sling-rand-varying PRIVATE -DALWAYS_REBUILD=1)
#
#add_executable(sling-rand-varying-gt ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_query_rand_pairs_varying_parameters.cpp ground_truth/random_pair_generator.cpp)
#InitSlingAP(sling-rand-varying-gt)
#target_compile_definitions(sling-rand-varying-gt PRIVATE -DALWAYS_REBUILD=1 -DGROUND_TRUTH=1)
#
############## playground
#add_subdirectory(playground)

