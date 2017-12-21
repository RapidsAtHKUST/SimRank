cmake_minimum_required(VERSION 3.6)
project(SPS)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

find_package(Boost REQUIRED program_options)
find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()
if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
endif ()

# compile for BLPMC
set(BLPMC_SOURCE_FILES bprw.cpp bprw.h util/file_serialization.h graph.cpp graph.h main.cpp util/stat.cpp util/stat.h ../SFMT/SFMT.c util/sfmt_based_rand.h)

add_executable(blpmc ${BLPMC_SOURCE_FILES})

target_include_directories(blpmc PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(blpmc PRIVATE ../sparsehash-yche/include)
target_include_directories(blpmc PRIVATE ../sparsepp)
target_include_directories(blpmc PRIVATE ../SFMT)
target_compile_options(blpmc PRIVATE -O3 -g -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)
target_compile_definitions(blpmc PRIVATE -DSFMT=1 -DDEBUG=1)
target_include_directories(blpmc PRIVATE ../include)

# compile for BLPMC-std
add_executable(blpmc-std ${BLPMC_SOURCE_FILES})

target_include_directories(blpmc-std PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(blpmc-std PRIVATE ../sparsehash-yche/include)
target_include_directories(blpmc-std PRIVATE ../sparsepp)
target_include_directories(blpmc-std PRIVATE ../SFMT)
target_include_directories(blpmc-std PRIVATE ../include)
target_compile_options(blpmc-std PRIVATE -O3 -g -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)
target_compile_definitions(blpmc-std PRIVATE -DDEBUG=1)

# compile for BLPMC-ap, all pair just for profiling
add_executable(blpmc-ap bprw.cpp bprw.h util/file_serialization.h graph.cpp graph.h playground/main_test_all_pair.cpp util/stat.cpp util/stat.h ../SFMT/SFMT.c)

target_include_directories(blpmc-ap PRIVATE ../sparsehash-yche/include)
target_include_directories(blpmc-ap PRIVATE ../sparsepp)
target_include_directories(blpmc-ap PRIVATE ../SFMT)
target_include_directories(blpmc-ap PRIVATE ${Boost_INCLUDE_DIRS})
target_compile_options(blpmc-ap PRIVATE -O3 -g -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)
target_compile_definitions(blpmc-ap PRIVATE -DSFMT=1)
target_include_directories(blpmc-ap PRIVATE ../include)

# compile for FLPMC
set(FLPMC_SOURCE_FILES util/file_serialization.h graph.cpp graph.h util/stat.cpp util/stat.h
        local_push.h local_push.cpp flpmc.cpp flpmc.h ../SFMT/SFMT.c
        simrank.cpp simrank.h)

add_executable(flpmc ${FLPMC_SOURCE_FILES})

target_include_directories(flpmc PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(flpmc PRIVATE ../sparsehash-yche/include)
target_include_directories(flpmc PRIVATE ../sparsepp)
target_include_directories(flpmc PRIVATE ../SFMT)
target_include_directories(flpmc PRIVATE ../include)
target_compile_options(flpmc PRIVATE -O3 -g -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)
target_link_libraries(flpmc PRIVATE ${Boost_LIBRARIES})
#target_compile_definitions(flpmc PRIVATE -DSFMT=1)

