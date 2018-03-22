cmake_minimum_required(VERSION 3.6)
project(pLP)

################################ 1st: basic environmental variables ######################################
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

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

################################ 3rd: source file environmental variables ######################################
set(UTIL_FILES util/stat.cpp util/stat.h util/graph_yche.cpp util/file_serialization.h util/thread_pool.h)
set(GROUND_TRUTH_FILES yche_refactor/simrank.cpp yche_refactor/simrank.h)
set(LPMC_FILES util/sparse_matrix_utils.h)
set(LP_FILES yche_refactor/local_push_yche.cpp yche_refactor/local_push_yche_rlp.cpp yche_refactor/local_push_yche.h)
set(PLP_FILES yche_refactor/parallel_local_push_yche.cpp yche_refactor/parallel_local_push_yche.h yche_refactor/parallel_local_push_yche_rlp.cpp)

############################## general macros ###################
macro(InitBoostHashSFMTEigen exec_name)
    target_include_directories(${exec_name} PRIVATE ${Boost_INCLUDE_DIRS})
    target_include_directories(${exec_name} PRIVATE ../sparsehash-yche/include)
    target_include_directories(${exec_name} PRIVATE ../sparsepp)
    target_include_directories(${exec_name} PRIVATE ../include)
endmacro()

macro(InitFLPMC exec_name)
    InitBoostHashSFMTEigen(${exec_name})
    target_link_libraries(${exec_name} PRIVATE ${Boost_LIBRARIES})
    target_link_libraries(${exec_name} PRIVATE tcmalloc)
    target_compile_options(${exec_name} PRIVATE -O3 -g -march=native)
    target_link_libraries(${exec_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    if (OPENMP_FOUND)
        target_compile_definitions(${exec_name} PRIVATE -DHAS_OPENMP=1)
    endif ()
endmacro()

####################### reduced local push testing
add_subdirectory(playground)
add_executable(rlp playground/main_exec_reduced_local_push.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${LP_FILES})
InitFLPMC(rlp)

add_executable(flp playground/main_exec_reduced_local_push.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${LP_FILES})
InitFLPMC(flp)
target_compile_definitions(flp PRIVATE -DFLP=1)


#########3
add_executable(pflp playground/main_exec_parallel_lp.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_FILES})
InitFLPMC(pflp)
target_compile_definitions(pflp PRIVATE -DFLP=1)

add_executable(prlp playground/main_exec_parallel_lp.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_FILES})
InitFLPMC(prlp)
