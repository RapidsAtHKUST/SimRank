cmake_minimum_required(VERSION 3.6)
project(LP_Parallel)

################################ 1st: basic environmental variables ######################################
set(CMAKE_CXX_STANDARD 11)

################################ 2nd: libarary environmental variables ######################################
find_package(Boost REQUIRED program_options)
if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
endif ()

find_package(Threads REQUIRED)

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

################################ 3rd: executable related ######################################
add_executable(pLP graph.cpp graph.h util/file_serialization.h
        local_push.cpp local_push.h util/simrank.h util/simrank.cpp util/stat.cpp util/stat.h main.cpp parallel_local_push.cpp )

target_compile_options(pLP PRIVATE -O3 -g)
target_include_directories(pLP PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(pLP PRIVATE ../sparsehash-yche/include)
target_include_directories(pLP PRIVATE ../sparsepp)
target_include_directories(pLP PRIVATE ../include)
target_link_libraries(pLP PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(pLP PRIVATE ${Boost_LIBRARIES})