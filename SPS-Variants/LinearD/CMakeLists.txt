cmake_minimum_required(VERSION 3.6)
project(LinSim)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost REQUIRED serialization program_options)

set(UTIL_FILES util/stat.cpp util/stat.h util/file_serialization.h util/node_pair.h util/graph.cpp util/graph.h)
set(SOURCE_FILES
        ${UTIL_FILES}
        lineard14/linearD.cpp lineard14/linearD.h tkde17/tkde17_sim.cpp tkde17/tkde17_sim.h lineard14/main_linearization.cpp)
set(CLOUD_WALKER_FILES ${UTIL_FILES}
        cloud_walker15/cloud_walker.cpp cloud_walker15/cloud_walker.h cloud_walker15/main_cloud_walker.cpp)

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

############################### release: CouldWalker #####################################################
add_executable(CloudWalker ${CLOUD_WALKER_FILES})
target_include_directories(CloudWalker PRIVATE ../../sparsehash-yche/include/)
target_include_directories(CloudWalker PRIVATE ../../sparsepp)
target_include_directories(CloudWalker PRIVATE ../../eigen)
target_include_directories(CloudWalker PRIVATE ../../include)
target_include_directories(CloudWalker PRIVATE ${Boost_INCLUDE_DIRS})

target_compile_options(CloudWalker PRIVATE -O3 ${SFMT_COMPILE_FLAGS})
target_link_libraries(CloudWalker PRIVATE ${Boost_LIBRARIES})