cmake_minimum_required(VERSION 3.6)
project(SPS)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost)
set(SOURCE_FILES bprw.cpp bprw.h file_serialization.h graph.cpp graph.h main.cpp stat.cpp stat.h)

add_executable(online-sps ${SOURCE_FILES})

if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
    target_include_directories(online-sps PRIVATE ${Boost_INCLUDE_DIRS})
endif ()
target_include_directories(online-sps PRIVATE ../sparsehash-c11)
target_include_directories(online-sps PRIVATE ../sparsepp)
target_include_directories(online-sps PRIVATE ../eigen)

target_compile_options(online-sps PRIVATE -O3)