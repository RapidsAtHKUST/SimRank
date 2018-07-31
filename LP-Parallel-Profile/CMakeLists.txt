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
set(PLP_FILES yche_refactor/parallel_local_push_yche_with_lock.cpp yche_refactor/parallel_local_push_yche_with_lock.h yche_refactor/parallel_local_push_yche_rlp_with_lock.cpp)
set(PLP_LOCK_FREE_FILES yche_refactor/parallel_local_push_yche.cpp yche_refactor/parallel_local_push_yche.h yche_refactor/parallel_local_push_yche_rlp_lock_free.cpp)
set(PPLP_FILES yche_refactor/thread_pool/parallel_local_push_yche_pthread.cpp yche_refactor/thread_pool/parallel_local_push_yche_pthread.h
        yche_refactor/thread_pool/parallel_local_push_yche_rlp_pthread.cpp)
set(LP_FILES yche_refactor/local_push_yche.cpp yche_refactor/local_push_yche_rlp.cpp yche_refactor/local_push_yche.h
        ${PLP_FILES})

############################## general macros ###################
macro(InitBoostHashSFMTEigen exec_name)
    target_include_directories(${exec_name} PRIVATE ${Boost_INCLUDE_DIRS})
    target_include_directories(${exec_name} PRIVATE ../dependencies/sparsehash-yche/include)
    target_include_directories(${exec_name} PRIVATE ../dependencies/sparsepp)
    target_include_directories(${exec_name} PRIVATE ../dependencies/include)
endmacro()

macro(InitFLPMC exec_name)
    InitBoostHashSFMTEigen(${exec_name})
    target_link_libraries(${exec_name} PRIVATE ${Boost_LIBRARIES})
    target_link_libraries(${exec_name} PRIVATE tcmalloc)
    target_compile_options(${exec_name} PRIVATE -O3 -g -march=native)
    #    target_compile_options(${exec_name} PRIVATE -O3 -g -Wcpp)
    target_link_libraries(${exec_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    if (OPENMP_FOUND)
        target_compile_definitions(${exec_name} PRIVATE -DHAS_OPENMP=1)
    endif ()
endmacro()

####################### reduced local push testing
#add_subdirectory(playground)

add_executable(rlp playground/main_exec_reduced_local_push.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${LP_FILES})
InitFLPMC(rlp)

add_executable(flp playground/main_exec_reduced_local_push.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${LP_FILES})
InitFLPMC(flp)
target_compile_definitions(flp PRIVATE -DFLP=1)


######### lock-based FLP/RLP
add_executable(pflp-with-lock playground/main_exec_parallel_lp_with_lock.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_FILES})
InitFLPMC(pflp-with-lock)
target_compile_definitions(pflp-with-lock PRIVATE -DFLP=1)

add_executable(prlp-with-lock playground/main_exec_parallel_lp_with_lock.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_FILES})
InitFLPMC(prlp-with-lock)
#target_compile_definitions(prlp-with-lock PRIVATE -DDEBUG=1)

add_executable(prlp-with-lock-stat playground/main_exec_parallel_lp_with_lock.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_FILES})
InitFLPMC(prlp-with-lock-stat)
target_compile_definitions(prlp-with-lock-stat PRIVATE -DPUSH_NUM_STAT=1)

######## lock-free FLP(not-supported)/RLP
add_executable(pflp-lock-free playground/main_exec_parallel_lp.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_LOCK_FREE_FILES})
InitFLPMC(pflp-lock-free)
target_compile_definitions(pflp-lock-free PRIVATE -DFLP=1)

add_executable(prlp-lock-free playground/main_exec_parallel_lp.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_LOCK_FREE_FILES})
InitFLPMC(prlp-lock-free)
#target_compile_definitions(prlp-lock-free PRIVATE -DDEBUG=1)

add_executable(prlp-lock-free-stat playground/main_exec_parallel_lp.cpp ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES} ${PLP_LOCK_FREE_FILES})
InitFLPMC(prlp-lock-free-stat)
target_compile_definitions(prlp-lock-free-stat PRIVATE -DPUSH_NUM_STAT=1)