cmake_minimum_required(VERSION 3.8)
project(tsf)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        input_output.h input_output.cpp
        config.h mytime.cpp mytime.h simrankmethod.h simrankvalue.hpp random.h
        tsf/gsinterface.h tsf/gsmanager.hpp tsf/rgsmanager.hpp tsf/rsamplegraph.hpp
        tsf/TSF.cpp tsf/TSF.h main.cpp)

add_executable(tsf ${SOURCE_FILES})
target_compile_options(tsf PRIVATE -O3 -g)