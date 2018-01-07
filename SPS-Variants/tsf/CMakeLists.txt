cmake_minimum_required(VERSION 3.8)
project(tsf)

set(CMAKE_CXX_STANDARD 11)

################ original tsf #############################
set(SOURCE_FILES
        input_output.h input_output.cpp
        util/pretty_print.h util/mytime.cpp util/mytime.h util/random.h
        tsf/rgsmanager.hpp tsf/rsamplegraph.hpp tsf/TSF.cpp tsf/TSF.h tsf/simrankmethod.h tsf/simrankvalue.hpp)

add_executable(tsf ${SOURCE_FILES} tsf/main.cpp)
target_compile_options(tsf PRIVATE -O3 -g)
target_compile_definitions(tsf PRIVATE SINGLE_SOURCE=1)

################## yche reimplementation of tsf #######################333
add_executable(tsf-spair main_single_pair.cpp yche_tsf.cpp yche_tsf.h input_output.cpp input_output.h)
target_compile_options(tsf-spair PRIVATE -O3 -g)

