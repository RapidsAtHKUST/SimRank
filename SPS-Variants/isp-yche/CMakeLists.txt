cmake_minimum_required(VERSION 3.8)
project(isp_yche)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost)

set(SOURCE_FILES main.cpp ISP.cpp ISP.h pretty_print.h)
add_executable(isp_yche ${SOURCE_FILES})
target_include_directories(isp_yche PRIVATE ../../sparsepp)
target_include_directories(isp_yche PRIVATE ${Boost_INCLUDE_DIRS})
target_compile_options(isp_yche PRIVATE -O3 -g)
