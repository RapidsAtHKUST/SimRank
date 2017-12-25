cmake_minimum_required(VERSION 3.6)
project(LPMC-Optimizing)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

find_package(Boost REQUIRED program_options)
if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
endif ()
find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

set(UTIL_FILES util/file_serialization.h util/stat.cpp util/stat.h
        util/sfmt_based_rand.h ../SFMT/SFMT.c util/graph_yche.cpp)
# util/graph_yche.cpp util/graph_yche.h
set(GROUND_TRUTH_FILES yche_refactor/simrank.cpp yche_refactor/simrank.h)
set(SFMT_COMPILE_FLAGS -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)

add_subdirectory(playground)

# 1st: compile for BLPMC
set(BLPMC_SOURCE_FILES yche_refactor/bprw_yche.cpp yche_refactor/bprw_yche.h
        ${UTIL_FILES} playground/main_bprw.cpp)

add_executable(blpmc ${BLPMC_SOURCE_FILES})

target_include_directories(blpmc PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(blpmc PRIVATE ../sparsehash-yche/include)
target_include_directories(blpmc PRIVATE ../sparsepp)
target_include_directories(blpmc PRIVATE ../SFMT)
target_include_directories(blpmc PRIVATE ../include)

target_link_libraries(blpmc PRIVATE tcmalloc)

target_compile_options(blpmc PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
target_compile_definitions(blpmc PRIVATE -DSFMT=1 -DDEBUG=1)

# 2nd: compile for BLPMC-std
add_executable(blpmc-std ${BLPMC_SOURCE_FILES})

target_include_directories(blpmc-std PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(blpmc-std PRIVATE ../sparsehash-yche/include)
target_include_directories(blpmc-std PRIVATE ../sparsepp)
target_include_directories(blpmc-std PRIVATE ../SFMT)
target_include_directories(blpmc-std PRIVATE ../include)

target_link_libraries(blpmc-std PRIVATE tcmalloc)

target_compile_options(blpmc-std PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
target_compile_definitions(blpmc-std PRIVATE -DDEBUG=1)

# 3rd: compile for BLPMC-ap, all pair just for profiling
add_executable(blpmc-ap yche_refactor/bprw_yche.cpp yche_refactor/bprw_yche.h
        ${UTIL_FILES} playground/main_bprw_all_pair.cpp)

target_include_directories(blpmc-ap PRIVATE ../sparsehash-yche/include)
target_include_directories(blpmc-ap PRIVATE ../sparsepp)
target_include_directories(blpmc-ap PRIVATE ../SFMT)
target_include_directories(blpmc-ap PRIVATE ${Boost_INCLUDE_DIRS})

target_include_directories(blpmc-ap PRIVATE ../include)

target_link_libraries(blpmc-ap PRIVATE tcmalloc)

target_compile_options(blpmc-ap PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
target_compile_definitions(blpmc-ap PRIVATE -DSFMT=1)

# 4th: compile for FLPMC
set(LPMC_FILES yche_refactor/flpmc_yche.h yche_refactor/flpmc_yche.cpp
        yche_refactor/local_push_yche.h yche_refactor/local_push_yche.cpp util/sparse_matrix_utils.h)
set(FLPMC_SOURCE_FILES ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES}
        playground/main_flpmc.cpp)

add_executable(flpmc ${FLPMC_SOURCE_FILES})

target_include_directories(flpmc PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(flpmc PRIVATE ../sparsehash-yche/include)
target_include_directories(flpmc PRIVATE ../sparsepp)
target_include_directories(flpmc PRIVATE ../SFMT)
target_include_directories(flpmc PRIVATE ../include)

target_link_libraries(flpmc PRIVATE ${Boost_LIBRARIES})
target_link_libraries(flpmc PRIVATE tcmalloc)

target_compile_options(flpmc PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
target_compile_definitions(flpmc PRIVATE -DSFMT=1)

# 5th: compile for FLPMC-ap
set(FLPMC_SOURCE_FILES ${LPMC_FILES} ${UTIL_FILES} ${GROUND_TRUTH_FILES}
        playground/main_flpmc_all_pair.cpp)

add_executable(flpmc-ap ${FLPMC_SOURCE_FILES})

target_include_directories(flpmc-ap PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(flpmc-ap PRIVATE ../sparsehash-yche/include)
target_include_directories(flpmc-ap PRIVATE ../sparsepp)
target_include_directories(flpmc-ap PRIVATE ../SFMT)
target_include_directories(flpmc-ap PRIVATE ../include)

target_link_libraries(flpmc-ap PRIVATE ${Boost_LIBRARIES})
target_link_libraries(flpmc-ap PRIVATE tcmalloc)

target_compile_options(flpmc-ap PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
target_compile_definitions(flpmc-ap PRIVATE -DSFMT=1)

# 6th: compile for BFLPMC
set(BFLPMC_SOURCE_FILES ${LPMC_FILES} yche_refactor/bprw_yche.cpp yche_refactor/bprw_yche.h
        yche_refactor/bflpmc_yche.h yche_refactor/bflpmc_yche.cpp
        ${UTIL_FILES} ${GROUND_TRUTH_FILES} playground/main_bflpmc.cpp)
add_executable(bflpmc ${BFLPMC_SOURCE_FILES})

target_include_directories(bflpmc PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(bflpmc PRIVATE ../sparsehash-yche/include)
target_include_directories(bflpmc PRIVATE ../sparsepp)
target_include_directories(bflpmc PRIVATE ../SFMT)
target_include_directories(bflpmc PRIVATE ../include)

target_link_libraries(bflpmc PRIVATE ${Boost_LIBRARIES})
target_link_libraries(bflpmc PRIVATE tcmalloc)

target_compile_options(bflpmc PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
target_compile_definitions(bflpmc PRIVATE -DSFMT=1)

# 7th: compile for BFLPMC-ap
set(BFLPMC_SOURCE_FILES ${LPMC_FILES} yche_refactor/bprw_yche.cpp yche_refactor/bprw_yche.h
        yche_refactor/bflpmc_yche.h yche_refactor/bflpmc_yche.cpp
        ${UTIL_FILES} ${GROUND_TRUTH_FILES} playground/main_bflpmc_all_pair.cpp)
add_executable(bflpmc-ap ${BFLPMC_SOURCE_FILES})

target_include_directories(bflpmc-ap PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(bflpmc-ap PRIVATE ../sparsehash-yche/include)
target_include_directories(bflpmc-ap PRIVATE ../sparsepp)
target_include_directories(bflpmc-ap PRIVATE ../SFMT)
target_include_directories(bflpmc-ap PRIVATE ../include)

target_link_libraries(bflpmc-ap PRIVATE ${Boost_LIBRARIES})
target_link_libraries(bflpmc-ap PRIVATE tcmalloc)

target_compile_options(bflpmc-ap PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
target_compile_definitions(bflpmc-ap PRIVATE -DSFMT=1)