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

set(SOURCE_FILES main.cpp ISP.cpp ISP.h pretty_print.h input_output.h input_output.cpp)
add_executable(isp_yche ${SOURCE_FILES})
target_include_directories(isp_yche PRIVATE ../../sparsepp)
target_include_directories(isp_yche PRIVATE ${Boost_INCLUDE_DIRS})
target_compile_options(isp_yche PRIVATE -O3 -g)
#target_compile_definitions(isp_yche PRIVATE DEBUG=1)

set(GROUND_TRUTH_FILES ground_truth/main_serial_naive_iter.cpp ground_truth/SerialNaiveSim.cpp ground_truth/SerialNaiveSim.h input_output.cpp)
add_executable(naive_iter ${GROUND_TRUTH_FILES})
target_include_directories(naive_iter PRIVATE ../../sparsepp)
target_compile_options(naive_iter PRIVATE -O3 -g)
#target_compile_definitions(naive_iter PRIVATE DEBUG=1)

set(REV_DIFFUSION_FILES ground_truth/main_serial_rev_naive_iter.cpp ground_truth/SerialRevNaiveSim.cpp ground_truth/SerialRevNaiveSim.h input_output.cpp)
add_executable(naive_rev_iter ${REV_DIFFUSION_FILES})
target_include_directories(naive_rev_iter PRIVATE ../../sparsepp)
target_compile_options(naive_rev_iter PRIVATE -O3 -g)
#target_compile_definitions(naive_rev_iter PRIVATE DEBUG=1)

set(REV_DIFFUSION_FILES ground_truth/main_serial_rev_naive_iter_query.cpp ground_truth/SerialRevNaiveSim.cpp ground_truth/SerialRevNaiveSim.h input_output.cpp)
add_executable(naive_rev_iter_sp ${REV_DIFFUSION_FILES})
target_include_directories(naive_rev_iter_sp PRIVATE ../../sparsepp)
target_compile_options(naive_rev_iter_sp PRIVATE -O3 -g)

add_subdirectory(playground)