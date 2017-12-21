cmake_minimum_required(VERSION 3.6)
project(SPS)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

find_package(Boost REQUIRED program_options)

# compile for BLPMC
set(BLPMC_SOURCE_FILES bprw.cpp bprw.h file_serialization.h graph.cpp graph.h main.cpp stat.cpp stat.h)

add_executable(blpmc ${BLPMC_SOURCE_FILES})

if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
    target_include_directories(blpmc PRIVATE ${Boost_INCLUDE_DIRS})
endif ()
target_include_directories(blpmc PRIVATE ../include)
target_include_directories(blpmc PRIVATE ../sparsehash-yche/include)
target_include_directories(blpmc PRIVATE ../sparsepp)
# comment out irrelevant dependency
# target_include_directories(blpmc PRIVATE ../eigen)
target_compile_options(blpmc PRIVATE -O3 -g)


# compile for FLPMC

set(FLPMC_SOURCE_FILES file_serialization.h graph.cpp graph.h stat.cpp stat.h local_push.h local_push.cpp flpmc.cpp flpmc.h simrank.h simrank.cpp)

add_executable(flpmc ${FLPMC_SOURCE_FILES})

if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
    target_include_directories(flpmc PRIVATE ${Boost_INCLUDE_DIRS})
endif ()
target_include_directories(flpmc PRIVATE ../include)
target_include_directories(flpmc PRIVATE ../sparsehash-yche/include)
target_include_directories(flpmc PRIVATE ../sparsepp)
# comment out irrelevant dependency

target_compile_options(flpmc PRIVATE -O3 -g)
target_link_libraries(flpmc PRIVATE ${Boost_LIBRARIES})
