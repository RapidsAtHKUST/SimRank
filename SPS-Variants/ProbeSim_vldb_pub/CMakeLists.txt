cmake_minimum_required(VERSION 3.10)
project(ProbeSim_vldb_pub)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

# setup pthread environmental variables
find_package(Threads)
find_package(Boost REQUIRED program_options)
if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
endif ()

set(GROUND_TRUTH_FILES ground_truth/simrank.cpp ground_truth/simrank.h ground_truth/graph_yche.cpp ground_truth/graph_yche.h
        ground_truth/sparse_matrix_utils.h ground_truth/file_serialization.h ground_truth/stat.h
        ground_truth/random_pair_generator.cpp ground_truth/random_pair_generator.h
        ground_truth/yche_serialization.h)

macro(InitHashSFMTEigenPthread exec_name)
    target_include_directories(${exec_name} PRIVATE ../../sparsehash-yche/include/)
    target_include_directories(${exec_name} PRIVATE ../../SFMT)
    target_include_directories(${exec_name} PRIVATE ../../include)
    target_include_directories(${exec_name} PRIVATE ../../sparsepp)

    target_link_libraries(${exec_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${exec_name} PRIVATE tcmalloc)

    target_compile_options(${exec_name} PRIVATE -O3 -g -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2 -DGROUND_TRUTH=1)
#    target_compile_options(${exec_name} PRIVATE -O3 -g -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)
endmacro()

add_executable(ProbeSim
        evaluate.h Graph.h Graph.cpp pooling.h Random.h SimStruct.h util.h main_varying_parameters.cpp
        ../../SFMT/SFMT.c ${GROUND_TRUTH_FILES})
#target_compile_options(ProbeSim PRIVATE -DDEBUG=1)
InitHashSFMTEigenPthread(ProbeSim)