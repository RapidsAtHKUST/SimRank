cmake_minimum_required(VERSION 3.6)
project(SPS)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

find_package(Boost REQUIRED program_options)

##################### Release #################################################################

# set(BPRW_SOURCES bprw.cpp simrank.cpp rw_hub.cpp stat.cpp file_serialization.h bflpmc.cpp flpmc.cpp)
set(BPRW_SOURCES stat.cpp rw_hub.cpp bprw.cpp simrank.cpp fgi.cpp)

add_executable(bflpmc ${BPRW_SOURCES} main.cpp)

if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
    target_include_directories(bflpmc PRIVATE ${Boost_INCLUDE_DIRS})
endif ()

# OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

macro(InitDirectories exec_name)
    target_include_directories(${exec_name} PRIVATE ./)
    target_include_directories(${exec_name} PRIVATE ../include)
    target_include_directories(${exec_name} PRIVATE ../sparsehash-yche/include)
    target_include_directories(${exec_name} PRIVATE ../sparsepp)
    target_include_directories(${exec_name} PRIVATE ../LPMC-Profile/)
    target_include_directories(${exec_name} PRIVATE ../SFMT/)
    target_include_directories(${exec_name} PRIVATE /homes/ywangby/workspace/DynamicSimRank/include/localsim)
    target_include_directories(${exec_name} PRIVATE /homes/ywangby/workspace/include)
endmacro()

# Cmake also process the cmakefile in the dir
add_subdirectory(/homes/ywangby/workspace/DynamicSimRank/include/localsim ./localsim)

set(SFMT_COMPILE_FLAGS -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)

add_subdirectory(../LPMC-Profile/util/ ./utils)

add_library(libphf SHARED IMPORTED)
set_property(TARGET libphf PROPERTY IMPORTED_LOCATION "/homes/ywangby/workspace/lib/libphf.so")

add_library(libbf SHARED IMPORTED)
set_property(TARGET libbf PROPERTY IMPORTED_LOCATION "/homes/ywangby/workspace/lib/libbf.so")

add_library(libroaring SHARED IMPORTED)
set_property(TARGET libroaring PROPERTY IMPORTED_LOCATION "/homes/ywangby/workspace/lib/libroaring.so")

macro(InitLibraries exec_name)
    target_link_libraries(${exec_name} PRIVATE libbf)
    target_link_libraries(${exec_name} PRIVATE libphf)
    target_link_libraries(${exec_name} PRIVATE libroaring)
    target_link_libraries(${exec_name} PRIVATE LocalPushSimRank)
    target_link_libraries(${exec_name} PRIVATE Utils)
    # target_link_libraries(${exec_name} PRIVATE mlpack armadillo) # for mlpack: a C++ machine learning library 
    target_compile_definitions(${exec_name} PRIVATE -DHAS_OPENMP=1 -DSFMT=1)
    target_compile_options(${exec_name} PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
    target_link_libraries(${exec_name} PRIVATE ${Boost_LIBRARIES})
endmacro()

InitDirectories(bflpmc)
InitLibraries(bflpmc)

add_executable(carmo ${BPRW_SOURCES} main_carmo_topk.cpp)
InitDirectories(carmo)
InitLibraries(carmo)

add_executable(carmo-h ${BPRW_SOURCES} main_carmo_h_topk.cpp)
InitDirectories(carmo-h)
InitLibraries(carmo-h)

add_executable(carmo-t ${BPRW_SOURCES} main_carmo_t_topk.cpp)
InitDirectories(carmo-t)
InitLibraries(carmo-t)

add_executable(blpmc ${BPRW_SOURCES} main_blpmc_topk.cpp)
InitDirectories(blpmc)
InitLibraries(blpmc)

#add_executable(flpmc flpmc.cpp stat.cpp main_flpmc_topk.cpp)
#add_executable(flpmc ${BPRW_SOURCES} main_flpmc_topk.cpp)
#InitDirectories(flpmc)
#InitLibraries(flpmc)

add_executable(blpmc-ap ${BPRW_SOURCES} main_blpmc.cpp)
InitDirectories(blpmc-ap)
InitLibraries(blpmc-ap)

add_executable(carmo-121 ${BPRW_SOURCES} main_carmo.cpp)
InitDirectories(carmo-121)
InitLibraries(carmo-121)

add_executable(carmo-h-121 ${BPRW_SOURCES} main_carmo_h.cpp)
InitDirectories(carmo-h-121)
InitLibraries(carmo-h-121)

add_executable(carmo-t-121 ${BPRW_SOURCES} main_carmo_t.cpp)
InitDirectories(carmo-t-121)
InitLibraries(carmo-t-121)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # add compile files for You Complete Me
set(CMAKE_BUILD_TYPE RelWithDebInfo) # optimized code with debug symbols


##################### Ground-Truth ################################################################

# set(GROUND_TRUTH_FILES file_serialization.h graph.cpp graph.h main_ground_truth_gen.cpp simrank.h simrank.cpp)
# add_executable(ground_truth_gen ${GROUND_TRUTH_FILES})
# target_include_directories(ground_truth_gen PRIVATE ${Boost_INCLUDE_DIRS})
# target_include_directories(ground_truth_gen PRIVATE ../include)
# target_include_directories(ground_truth_gen PRIVATE ../sparsehash-yche/include)
# target_include_directories(ground_truth_gen PRIVATE ../sparsepp)
# target_compile_options(ground_truth_gen PRIVATE -O3 -g)


