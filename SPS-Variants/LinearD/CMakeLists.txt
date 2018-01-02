cmake_minimum_required(VERSION 3.6)
project(LinSim)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost REQUIRED serialization program_options)

set(SOURCE_FILES
        util/stat.cpp util/stat.h util/file_serialization.h graph.cpp graph.h util/node_pair.h
        linearD.cpp linearD.h tkde17/tkde17_sim.cpp tkde17/tkde17_sim.h
        main_linearization.cpp)


set(SFMT_COMPILE_FLAGS -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)

############################### release: LinSim #####################################################
add_executable(LinSim ${SOURCE_FILES})
target_include_directories(LinSim PRIVATE ../../sparsehash-yche/include/)
target_include_directories(LinSim PRIVATE ../../sparsepp)
target_include_directories(LinSim PRIVATE ../../eigen)
target_include_directories(LinSim PRIVATE ../../include)
target_include_directories(LinSim PRIVATE ${Boost_INCLUDE_DIRS})

target_compile_options(LinSim PRIVATE -O3 ${SFMT_COMPILE_FLAGS})
target_link_libraries(LinSim PRIVATE ${Boost_LIBRARIES})