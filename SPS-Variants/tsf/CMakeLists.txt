cmake_minimum_required(VERSION 3.8)
project(tsf)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

set(SFMT_COMPILE_FLAGS -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)
################ original tsf #############################
set(SOURCE_FILES
        input_output.h input_output.cpp
        util/pretty_print.h util/mytime.cpp util/mytime.h util/random.h
        tsf/rgsmanager.hpp tsf/rsamplegraph.hpp tsf/TSF.cpp tsf/TSF.h tsf/simrankmethod.h tsf/simrankvalue.hpp)

add_executable(tsf ${SOURCE_FILES} tsf/main.cpp)

target_compile_options(tsf PRIVATE ${SFMT_COMPILE_FLAGS} -O3 -g)
target_compile_definitions(tsf PRIVATE SINGLE_SOURCE=1)

################## yche reimplementation of tsf #######################
add_executable(tsf-spair main_single_pair.cpp yche_tsf.cpp yche_tsf.h input_output.cpp input_output.h ../../SFMT/SFMT.c)
target_include_directories(tsf-spair PRIVATE ../../SFMT)

target_compile_options(tsf-spair PRIVATE -O3 -g)

################## yche reimplementation of tsf #######################
macro(InitTSF exec_name)
    target_include_directories(${exec_name} PRIVATE ../../sparsepp)
    target_include_directories(${exec_name} PRIVATE ../../include)
    target_include_directories(${exec_name} PRIVATE ../../sparsehash-yche/include)
    target_include_directories(${exec_name} PRIVATE ../../SFMT)
    target_link_libraries(${exec_name} PRIVATE tcmalloc)
    target_compile_options(${exec_name} PRIVATE ${SFMT_COMPILE_FLAGS} -O3 -g)
endmacro()

###### tsf-yche(ground-truth)
set(GROUND_TRUTH_FILES ground_truth/simrank.cpp ground_truth/simrank.h ground_truth/graph_yche.cpp ground_truth/graph_yche.h
        ground_truth/sparse_matrix_utils.h ground_truth/file_serialization.h ground_truth/stat.cpp ground_truth/stat.h)
add_executable(tsf-ap main_all_pair_query.cpp
        yche_tsf.cpp yche_tsf.h input_output.cpp input_output.h ${GROUND_TRUTH_FILES} ../../SFMT/SFMT.c util/sfmt_based_rand.h)
InitTSF(tsf-ap)
target_compile_definitions(tsf-ap PRIVATE -DGROUND_TRUTH=1)

#### tsf-yche(benchmark)

add_executable(tsf-bench main_all_pair_query.cpp
        yche_tsf.cpp yche_tsf.h input_output.cpp input_output.h ${GROUND_TRUTH_FILES} ../../SFMT/SFMT.c util/sfmt_based_rand.h)
InitTSF(tsf-bench)

#### tsf-yche(random pairs)
add_executable(tsf-rand-bench main_rand_pair_query.cpp ground_truth/random_pair_generator.cpp
        yche_tsf.cpp yche_tsf.h input_output.cpp input_output.h ${GROUND_TRUTH_FILES} ../../SFMT/SFMT.c util/sfmt_based_rand.h)
InitTSF(tsf-rand-bench)

add_executable(tsf-rand-bench-gt main_rand_pair_query.cpp ground_truth/random_pair_generator.cpp
        yche_tsf.cpp yche_tsf.h input_output.cpp input_output.h ${GROUND_TRUTH_FILES} ../../SFMT/SFMT.c util/sfmt_based_rand.h)
InitTSF(tsf-rand-bench-gt)
target_compile_definitions(tsf-rand-bench-gt PRIVATE -DGROUND_TRUTH=1)

#### tsf-yche (random pairs varying parameters)
add_executable(tsf-rand-varying main_rand_pair_query_varying_parameters.cpp ground_truth/random_pair_generator.cpp
        yche_tsf.cpp yche_tsf.h input_output.cpp input_output.h ${GROUND_TRUTH_FILES} ../../SFMT/SFMT.c util/sfmt_based_rand.h)
InitTSF(tsf-rand-varying)

add_executable(tsf-rand-varying-gt main_rand_pair_query_varying_parameters.cpp ground_truth/random_pair_generator.cpp
        yche_tsf.cpp yche_tsf.h input_output.cpp input_output.h ${GROUND_TRUTH_FILES} ../../SFMT/SFMT.c util/sfmt_based_rand.h)
InitTSF(tsf-rand-varying-gt)
target_compile_definitions(tsf-rand-varying-gt PRIVATE -DGROUND_TRUTH=1)

