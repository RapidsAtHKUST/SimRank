cmake_minimum_required(VERSION 3.6)
project(sling)

set(CMAKE_CXX_STANDARD 11)

set(GROUND_TRUTH_FILES ground_truth/simrank.cpp ground_truth/simrank.h ground_truth/graph_yche.cpp ground_truth/graph_yche.h
        ground_truth/sparse_matrix_utils.h ground_truth/file_serialization.h ground_truth/stat.cpp ground_truth/stat.h)
set(SOURCE_FILES graph.h sling.h sling.cpp ../../SFMT/SFMT.c input_output.cpp input_output.h util.h util.cpp)

# setup pthread environmental variables
find_package(Threads REQUIRED)
find_package(OpenMP)

if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

# 1st: sling single pair
add_executable(sling ${SOURCE_FILES} main.cpp)

target_include_directories(sling PRIVATE ../../sparsehash-yche/include/)
target_include_directories(sling PRIVATE ../../SFMT)
target_include_directories(sling PRIVATE ../../include)

target_link_libraries(sling PRIVATE ${CMAKE_THREAD_LIBS_INIT})

target_compile_options(sling PRIVATE -O3 -g -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)

# 2nd: sling all pair
add_executable(sling_all ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_query_all_vertices.cpp)

target_include_directories(sling_all PRIVATE ../../sparsehash-yche/include/)
target_include_directories(sling_all PRIVATE ../../SFMT)
target_include_directories(sling_all PRIVATE ../../include)
target_include_directories(sling_all PRIVATE ../../sparsepp)

target_link_libraries(sling_all PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(sling_all PRIVATE tcmalloc)

target_compile_options(sling_all PRIVATE -O3 -g -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)
target_compile_definitions(sling_all PRIVATE -DGROUND_TRUTH=1)