cmake_minimum_required(VERSION 3.8)
project(par-sr)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

set(SFMT_COMPILE_FLAGS -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)

macro(Init exec_name)
    target_include_directories(${exec_name} PRIVATE ../../sparsepp)
    target_include_directories(${exec_name} PRIVATE ../../include)
    target_include_directories(${exec_name} PRIVATE ../../sparsehash-yche/include)
    target_include_directories(${exec_name} PRIVATE ../../SFMT)
    target_link_libraries(${exec_name} PRIVATE tcmalloc)
    target_compile_options(${exec_name} PRIVATE ${SFMT_COMPILE_FLAGS} -O3 -g)
endmacro()

set(GROUND_TRUTH_FILES ground_truth/simrank.cpp ground_truth/simrank.h ground_truth/graph_yche.cpp ground_truth/graph_yche.h
    ground_truth/sparse_matrix_utils.h ground_truth/file_serialization.h ground_truth/stat.cpp ground_truth/stat.h)
add_executable(par-sr ground_truth/random_pair_generator.cpp ${GROUND_TRUTH_FILES} par-sr.cpp)
Init(par-sr)
target_compile_options(par-sr PRIVATE -O3)

