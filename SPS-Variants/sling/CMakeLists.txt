cmake_minimum_required(VERSION 3.6)
project(sling)

set(CMAKE_CXX_STANDARD 11)

set(GROUND_TRUTH_FILES ground_truth/simrank.cpp ground_truth/simrank.h ground_truth/graph_yche.cpp ground_truth/graph_yche.h
        ground_truth/sparse_matrix_utils.h ground_truth/file_serialization.h ground_truth/stat.h
        ground_truth/yche_serialization.h)
set(SOURCE_FILES graph.h sling.h sling.cpp ../../SFMT/SFMT.c input_output.cpp input_output.h util.h util.cpp ground_truth/stat.cpp)

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
    target_include_directories(${exec_name} PRIVATE ../../sparsehash-yche/include/)
    target_include_directories(${exec_name} PRIVATE ../../SFMT)
    target_include_directories(${exec_name} PRIVATE ../../include)

    target_link_libraries(${exec_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${exec_name} PRIVATE tcmalloc)

    target_compile_options(${exec_name} PRIVATE -O3 -g -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)
endmacro()

# 1st: sling single pair
add_executable(sling ${SOURCE_FILES} main.cpp)
InitHashSFMTEigenPthread(sling)

# 2nd: sling all pair
macro(InitSlingAP exec_name)
    target_include_directories(${exec_name} PRIVATE ../../sparsepp)
    InitHashSFMTEigenPthread(${exec_name})
endmacro()

add_executable(sling_all ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_query_all_vertices.cpp)
InitSlingAP(sling_all)
target_compile_definitions(sling_all PRIVATE -DGROUND_TRUTH=1 -DALL_PAIR=1)
#target_compile_definitions(sling_all PRIVATE -DGROUND_TRUTH=1)

########## benchmark
add_executable(sling_bench ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_query_all_vertices.cpp)
InitSlingAP(sling_bench)

####
add_executable(sling-rand-bench ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_query_rand_pairs.cpp ground_truth/random_pair_generator.cpp)
InitSlingAP(sling-rand-bench)

add_executable(sling-rand-bench-gt ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_query_rand_pairs.cpp ground_truth/random_pair_generator.cpp)
InitSlingAP(sling-rand-bench-gt)
target_compile_definitions(sling-rand-bench-gt PRIVATE -DGROUND_TRUTH=1)

############# playground
add_subdirectory(playground)

