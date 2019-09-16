cmake_minimum_required(VERSION 3.10)
project(ProbeSim_vldb_pub)

set(CMAKE_CXX_STANDARD 11)

add_compile_options(-Wall -O3 -g -march=native)
add_definitions(-DUSE_LOG -DTIME_RESULTS)
include_directories(${CMAKE_SOURCE_DIR})

# setup pthread environmental variables
find_package(Threads)
find_package(OpenMP)

if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()
find_package(Boost REQUIRED program_options)
if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
endif ()

set(FAKE_FILES ground_truth/simrank.h ground_truth/graph_yche.h ground_truth/sparse_matrix_utils.h
        ground_truth/file_serialization.h ground_truth/stat.h ground_truth/random_pair_generator.h
        ground_truth/yche_serialization.h
        evaluate.h Graph.h pooling.h Random.h SimStruct.h util.h)

set(GROUND_TRUTH_FILES ground_truth/simrank.cpp ground_truth/graph_yche.cpp ground_truth/random_pair_generator.cpp
        util/log.cpp)

macro(InitHashSFMTEigenPthread exec_name)
    target_include_directories(${exec_name} PRIVATE ../../dependencies/sparsehash-yche/include/)
    target_include_directories(${exec_name} PRIVATE ../../dependencies/SFMT)
    target_include_directories(${exec_name} PRIVATE ../../dependencies/include)
    target_include_directories(${exec_name} PRIVATE ../../dependencies/sparsepp)

    target_link_libraries(${exec_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${exec_name} PRIVATE tcmalloc)

    target_compile_options(${exec_name} PRIVATE -O3 -g -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)
    #    target_compile_options(${exec_name} PRIVATE -O3 -g -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)
endmacro()


# with gt
add_executable(ProbeSim-gt
        Graph.cpp main_varying_parameters.cpp ../../dependencies/SFMT/SFMT.c ${GROUND_TRUTH_FILES})
target_compile_options(ProbeSim-gt PRIVATE -DGROUND_TRUTH=1)
InitHashSFMTEigenPthread(ProbeSim-gt)

# without gt
add_executable(ProbeSim
        Graph.cpp main_varying_parameters.cpp ../../dependencies/SFMT/SFMT.c ${GROUND_TRUTH_FILES})
InitHashSFMTEigenPthread(ProbeSim)

add_executable(probesim_ss_ap
        Graph.cpp main_ss_ap.cpp ../../dependencies/SFMT/SFMT.c ${GROUND_TRUTH_FILES})
InitHashSFMTEigenPthread(probesim_ss_ap)