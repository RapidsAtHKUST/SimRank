cmake_minimum_required(VERSION 3.6)
project(simpush_release_code)

## Show Compilation Commands
option(DUMP_COMPILATION_TO_FILE "dump compilation to files" ON)
if (NOT DUMP_COMPILATION_TO_FILE)
    set(CMAKE_VERBOSE_MAKEFILE ON)
else ()
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # dump compilation commands to compile_commands.json
endif ()
set(CMAKE_CXX_STANDARD 14)
add_compile_options(-g -O3 -march=native -DSFMT_MEXP=607 -DUSE_LOG=1)
include_directories(.)
include_directories(SFMT-src-1.4.1)

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

add_executable(simpush_release_code
        #        SFMT-src-1.4.1/SFMT.c
        util/log.cpp
        SFMT.c
        main.cpp
        )

add_executable(eval
        #        SFMT-src-1.4.1/SFMT.c
        util/log.cpp
        SFMT.c
        cal_evalaute.cpp
        )

add_executable(simpush
        util/log.cpp
        SFMT.c
        main_ss_yche.cpp)