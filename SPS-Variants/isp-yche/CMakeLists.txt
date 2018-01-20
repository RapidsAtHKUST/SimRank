cmake_minimum_required(VERSION 3.8)
project(isp_yche)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost)

find_package(OpenMP)

if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

set(GROUND_TRUTH_FILES ground_truth/simrank.cpp ground_truth/simrank.h ground_truth/graph_yche.cpp ground_truth/graph_yche.h
        ground_truth/sparse_matrix_utils.h ground_truth/file_serialization.h ground_truth/stat.cpp ground_truth/stat.h)

set(SOURCE_FILES ISP.cpp ISP.h pretty_print.h input_output.h input_output.cpp)
add_executable(isp_yche ${SOURCE_FILES} main.cpp)
target_include_directories(isp_yche PRIVATE ../../sparsepp)
target_include_directories(isp_yche PRIVATE ${Boost_INCLUDE_DIRS})
target_compile_options(isp_yche PRIVATE -O3 -g)
#target_compile_definitions(isp_yche PRIVATE DEBUG=1)

#### isp ground truth testing
macro(InitISP exec_name)
    target_include_directories(${exec_name} PRIVATE ../../sparsepp)
    target_include_directories(${exec_name} PRIVATE ${Boost_INCLUDE_DIRS})
    target_include_directories(${exec_name} PRIVATE ../../include)
    target_include_directories(${exec_name} PRIVATE ../../sparsehash-yche/include)

    target_compile_options(${exec_name} PRIVATE -O3 -g)
    target_link_libraries(${exec_name} tcmalloc)
endmacro()

add_executable(isp_ap ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_all_pair.cpp)
InitISP(isp_ap)
target_compile_definitions(isp_ap PRIVATE -DGROUND_TRUTH=1)

#### isp benchmarking
add_executable(isp_bench ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_all_pair.cpp)
InitISP(isp_bench)

#### isp(random pairs)
add_executable(isp-rand-bench ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_rand_pair.cpp ground_truth/random_pair_generator.cpp)
InitISP(isp-rand-bench)

add_executable(isp-rand-bench-gt ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_rand_pair.cpp ground_truth/random_pair_generator.cpp)
InitISP(isp-rand-bench-gt)
target_compile_definitions(isp-rand-bench-gt PRIVATE -DGROUND_TRUTH=1)

#### isp(random paris - accept varying parameters)
add_executable(isp-rand-varying-gt ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_rand_pair_varying_parameters.cpp ground_truth/random_pair_generator.cpp)
InitISP(isp-rand-varying-gt)
target_compile_definitions(isp-rand-varying-gt PRIVATE -DGROUND_TRUTH=1)

add_executable(isp-rand-varying ${SOURCE_FILES} ${GROUND_TRUTH_FILES} main_rand_pair_varying_parameters.cpp ground_truth/random_pair_generator.cpp)
InitISP(isp-rand-varying)

############### others ######################################################
set(YCHE_GROUND_TRUTH_FILES yche_ground_truth/main_serial_naive_iter.cpp yche_ground_truth/SerialNaiveSim.cpp yche_ground_truth/SerialNaiveSim.h input_output.cpp)
add_executable(naive_iter ${YCHE_GROUND_TRUTH_FILES})
target_include_directories(naive_iter PRIVATE ../../sparsepp)
target_compile_options(naive_iter PRIVATE -O3 -g)
#target_compile_definitions(naive_iter PRIVATE DEBUG=1)

set(REV_DIFFUSION_FILES yche_ground_truth/main_serial_rev_naive_iter.cpp yche_ground_truth/SerialRevNaiveSim.cpp yche_ground_truth/SerialRevNaiveSim.h input_output.cpp)
add_executable(naive_rev_iter ${REV_DIFFUSION_FILES})
target_include_directories(naive_rev_iter PRIVATE ../../sparsepp)
target_compile_options(naive_rev_iter PRIVATE -O3 -g)
#target_compile_definitions(naive_rev_iter PRIVATE DEBUG=1)

set(REV_DIFFUSION_FILES yche_ground_truth/main_serial_rev_naive_iter_query.cpp yche_ground_truth/SerialRevNaiveSim.cpp yche_ground_truth/SerialRevNaiveSim.h input_output.cpp)
add_executable(naive_rev_iter_sp ${REV_DIFFUSION_FILES})
target_include_directories(naive_rev_iter_sp PRIVATE ../../sparsepp)
target_compile_options(naive_rev_iter_sp PRIVATE -O3 -g)

add_subdirectory(playground)