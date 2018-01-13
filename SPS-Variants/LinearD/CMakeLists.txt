cmake_minimum_required(VERSION 3.6)
project(LinSim)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost REQUIRED serialization program_options)

find_package(OpenMP)

if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

####################### related source files #######################
set(GROUND_TRUTH_FILES ground_truth/simrank.cpp ground_truth/simrank.h ground_truth/graph_yche.cpp ground_truth/graph_yche.h
        ground_truth/sparse_matrix_utils.h ground_truth/file_serialization.h ground_truth/stat.cpp ground_truth/stat.h)

set(UTIL_FILES util/node_pair.h util/graph.cpp util/graph.h ground_truth/stat.cpp)
set(SFMT_COMPILE_FLAGS -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)

############################### release: LinSim #####################################################
set(SOURCE_FILES ${UTIL_FILES} lineard14/linearD.cpp lineard14/linearD.h tkde17/tkde17_sim.cpp tkde17/tkde17_sim.h)

add_executable(LinSim ${SOURCE_FILES} lineard14/main_linearization.cpp)
target_include_directories(LinSim PRIVATE ../../sparsehash-yche/include/)
target_include_directories(LinSim PRIVATE ../../sparsepp)
target_include_directories(LinSim PRIVATE ../../eigen)
target_include_directories(LinSim PRIVATE ../../include)
target_include_directories(LinSim PRIVATE ${Boost_INCLUDE_DIRS})

target_compile_options(LinSim PRIVATE -O3 ${SFMT_COMPILE_FLAGS})
target_link_libraries(LinSim PRIVATE ${Boost_LIBRARIES})

######################## release: LinSimAP ##########################################
add_executable(LinSimAP ${SOURCE_FILES} lineard14/main_lineard_ap.cpp ${GROUND_TRUTH_FILES})
target_include_directories(LinSimAP PRIVATE ../../sparsehash-yche/include/)
target_include_directories(LinSimAP PRIVATE ../../sparsepp)
target_include_directories(LinSimAP PRIVATE ../../eigen)
target_include_directories(LinSimAP PRIVATE ../../include)
target_include_directories(LinSimAP PRIVATE ${Boost_INCLUDE_DIRS})

target_compile_options(LinSimAP PRIVATE -O3 ${SFMT_COMPILE_FLAGS})
target_compile_definitions(LinSimAP PRIVATE -DGROUND_TRUTH=1)
target_link_libraries(LinSimAP PRIVATE ${Boost_LIBRARIES})
target_link_libraries(LinSimAP PRIVATE tcmalloc)

####################### LinSimBench

add_executable(LinSimBench ${SOURCE_FILES} lineard14/main_lineard_ap.cpp ${GROUND_TRUTH_FILES})
target_include_directories(LinSimBench PRIVATE ../../sparsehash-yche/include/)
target_include_directories(LinSimBench PRIVATE ../../sparsepp)
target_include_directories(LinSimBench PRIVATE ../../eigen)
target_include_directories(LinSimBench PRIVATE ../../include)
target_include_directories(LinSimBench PRIVATE ${Boost_INCLUDE_DIRS})

target_compile_options(LinSimBench PRIVATE -O3 ${SFMT_COMPILE_FLAGS})
target_link_libraries(LinSimBench PRIVATE ${Boost_LIBRARIES})
target_link_libraries(LinSimBench PRIVATE tcmalloc)

############################### release: CloudWalker #####################################################
set(CLOUD_WALKER_FILES ${UTIL_FILES}
        cloud_walker15/cloud_walker.cpp cloud_walker15/cloud_walker.h)
add_executable(CloudWalker ${CLOUD_WALKER_FILES} cloud_walker15/main_cloud_walker.cpp)
target_include_directories(CloudWalker PRIVATE ../../sparsehash-yche/include/)
target_include_directories(CloudWalker PRIVATE ../../sparsepp)
target_include_directories(CloudWalker PRIVATE ../../eigen)
target_include_directories(CloudWalker PRIVATE ../../include)
target_include_directories(CloudWalker PRIVATE ${Boost_INCLUDE_DIRS})

target_compile_options(CloudWalker PRIVATE -O3 ${SFMT_COMPILE_FLAGS})
target_link_libraries(CloudWalker PRIVATE ${Boost_LIBRARIES})

############################# release: CloudWalkerAP ###########################
add_executable(CloudWalkerAP ${CLOUD_WALKER_FILES} cloud_walker15/main_cloud_ap.cpp ${GROUND_TRUTH_FILES})
target_include_directories(CloudWalkerAP PRIVATE ../../sparsehash-yche/include/)
target_include_directories(CloudWalkerAP PRIVATE ../../sparsepp)
target_include_directories(CloudWalkerAP PRIVATE ../../eigen)
target_include_directories(CloudWalkerAP PRIVATE ../../include)
target_include_directories(CloudWalkerAP PRIVATE ${Boost_INCLUDE_DIRS})

target_compile_options(CloudWalkerAP PRIVATE -O3 ${SFMT_COMPILE_FLAGS})
target_compile_definitions(CloudWalkerAP PRIVATE -DGROUND_TRUTH=1)
target_link_libraries(CloudWalkerAP PRIVATE ${Boost_LIBRARIES})
target_link_libraries(CloudWalkerAP PRIVATE tcmalloc)

############ cloud-walker-bench

add_executable(CloudWalkerBench ${CLOUD_WALKER_FILES} cloud_walker15/main_cloud_ap.cpp ${GROUND_TRUTH_FILES})
target_include_directories(CloudWalkerBench PRIVATE ../../sparsehash-yche/include/)
target_include_directories(CloudWalkerBench PRIVATE ../../sparsepp)
target_include_directories(CloudWalkerBench PRIVATE ../../eigen)
target_include_directories(CloudWalkerBench PRIVATE ../../include)
target_include_directories(CloudWalkerBench PRIVATE ${Boost_INCLUDE_DIRS})

target_compile_options(CloudWalkerBench PRIVATE -O3 ${SFMT_COMPILE_FLAGS})
target_link_libraries(CloudWalkerBench PRIVATE ${Boost_LIBRARIES})
target_link_libraries(CloudWalkerBench PRIVATE tcmalloc)
