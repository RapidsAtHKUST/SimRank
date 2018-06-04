cmake_minimum_required(VERSION 3.6)
project(SPS)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

find_package(Boost REQUIRED program_options)

##################### Release #################################################################

aux_source_directory(. DIR_SRCS) 

add_executable(bflpmc ${DIR_SRCS})

if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
    target_include_directories(bflpmc PRIVATE ${Boost_INCLUDE_DIRS})
endif ()

# OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
target_include_directories(bflpmc PRIVATE ./)
target_include_directories(bflpmc PRIVATE ../include)
target_include_directories(bflpmc PRIVATE ../sparsehash-yche/include)
target_include_directories(bflpmc PRIVATE ../sparsepp)
target_include_directories(bflpmc PRIVATE /homes/ywangby/workspace/DynamicSimRank/include/localsim)

# Cmake also process the cmakefile in the dir
add_subdirectory(/homes/ywangby/workspace/DynamicSimRank/include/localsim ./localsim)

add_subdirectory(../LPMC-Profile/util/ ./utils)

target_link_libraries(bflpmc PRIVATE LocalPushSimRank)

target_link_libraries(bflpmc PRIVATE Utils)

# target_link_libraries(bflpmc PRIVATE mlpack armadillo) # for mlpack: a C++ machine learning library 

target_compile_options(bflpmc PRIVATE -O3 -g)
target_link_libraries(bflpmc PRIVATE ${Boost_LIBRARIES})


set( CMAKE_EXPORT_COMPILE_COMMANDS ON) # add compile files for You Complete Me

##################### Ground-Truth ################################################################

# set(GROUND_TRUTH_FILES file_serialization.h graph.cpp graph.h main_ground_truth_gen.cpp simrank.h simrank.cpp)
# add_executable(ground_truth_gen ${GROUND_TRUTH_FILES})
# target_include_directories(ground_truth_gen PRIVATE ${Boost_INCLUDE_DIRS})
# target_include_directories(ground_truth_gen PRIVATE ../include)
# target_include_directories(ground_truth_gen PRIVATE ../sparsehash-yche/include)
# target_include_directories(ground_truth_gen PRIVATE ../sparsepp)
# target_compile_options(ground_truth_gen PRIVATE -O3 -g)
