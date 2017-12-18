cmake_minimum_required(VERSION 3.8)
project(tsf)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        input_output.h input_output.cpp
        util/pretty_print.h util/mytime.cpp util/mytime.h tsf/simrankmethod.h tsf/simrankvalue.hpp util/random.h
        tsf/rgsmanager.hpp tsf/rsamplegraph.hpp tsf/TSF.cpp tsf/TSF.h)

add_executable(tsf ${SOURCE_FILES} main.cpp)
target_compile_options(tsf PRIVATE -O3 -g)
target_compile_definitions(tsf PRIVATE SINGLE_SOURCE=1)

add_executable(tsf-spair ${SOURCE_FILES} main_single_pair.cpp yche_tsf.cpp yche_tsf.h)
target_compile_options(tsf-spair PRIVATE -O3 -g)

