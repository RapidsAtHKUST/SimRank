cmake_minimum_required(VERSION 3.6)
project(SPS)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

find_package(Boost REQUIRED program_options)

##################### Release #################################################################

set(BFLPMC_SOURCE_FILES file_serialization.h graph.cpp graph.h stat.cpp stat.h local_push.h local_push.cpp flpmc.cpp flpmc.h simrank.h simrank.cpp bflpmc.cpp bflpmc.h main.cpp bprw.h bprw.cpp)

add_executable(bflpmc ${BFLPMC_SOURCE_FILES})

if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
    target_include_directories(bflpmc PRIVATE ${Boost_INCLUDE_DIRS})
endif ()
target_include_directories(bflpmc PRIVATE ../include)
target_include_directories(bflpmc PRIVATE ../sparsehash-yche/include)
target_include_directories(bflpmc PRIVATE ../sparsepp)

target_compile_options(bflpmc PRIVATE -O3 -g)
target_link_libraries(bflpmc PRIVATE ${Boost_LIBRARIES})

##################### Ground-Truth ################################################################

set(GROUND_TRUTH_FILES file_serialization.h graph.cpp graph.h main_ground_truth_gen.cpp simrank.h simrank.cpp)
add_executable(ground_truth_gen ${GROUND_TRUTH_FILES})
target_include_directories(ground_truth_gen PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(ground_truth_gen PRIVATE ../include)
target_include_directories(ground_truth_gen PRIVATE ../sparsehash-yche/include)
target_include_directories(ground_truth_gen PRIVATE ../sparsepp)
target_compile_options(ground_truth_gen PRIVATE -O3 -g)
