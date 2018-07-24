cmake_minimum_required(VERSION 3.6)
project(LinSim)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost REQUIRED serialization program_options filesystem)
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

############################## general macros ###################
macro(InitBoostHashSFMTEigen exec_name)
    target_include_directories(${exec_name} PRIVATE ${Boost_INCLUDE_DIRS})
    target_include_directories(${exec_name} PRIVATE ../../dependencies/sparsehash-yche/include)
    target_include_directories(${exec_name} PRIVATE ../../dependencies/sparsepp)
    target_include_directories(${exec_name} PRIVATE ../../dependencies/include)

    target_compile_options(${exec_name} PRIVATE -O3)
    target_link_libraries(${exec_name} PRIVATE ${Boost_LIBRARIES})
    target_link_libraries(${exec_name} PRIVATE tcmalloc)
endmacro()

############################### release: LinSim #####################################################
set(SOURCE_FILES ${UTIL_FILES} lineard14/linearD.cpp lineard14/linearD.h)

add_executable(LinSim ${SOURCE_FILES} lineard14/main_linearization.cpp)
InitBoostHashSFMTEigen(LinSim)

#### LinSimAP
add_executable(LinSimAP ${SOURCE_FILES} lineard14/main_lineard_ap.cpp ${GROUND_TRUTH_FILES})
InitBoostHashSFMTEigen(LinSimAP)
target_compile_definitions(LinSimAP PRIVATE -DGROUND_TRUTH=1)

#### LinSimBench
add_executable(LinSimBench ${SOURCE_FILES} lineard14/main_lineard_ap.cpp ${GROUND_TRUTH_FILES})
InitBoostHashSFMTEigen(LinSimBench)

#### lind(random pairs)
add_executable(lind-rand-ben ${SOURCE_FILES} lineard14/main_lineard_rand_pairs.cpp ${GROUND_TRUTH_FILES} ground_truth/random_pair_generator.cpp)
InitBoostHashSFMTEigen(lind-rand-ben)

add_executable(lind-rand-ben-gt ${SOURCE_FILES} lineard14/main_lineard_rand_pairs.cpp ${GROUND_TRUTH_FILES} ground_truth/random_pair_generator.cpp)
InitBoostHashSFMTEigen(lind-rand-ben-gt)
target_compile_definitions(lind-rand-ben-gt PRIVATE -DGROUND_TRUTH=1)

############################### release: CloudWalker #####################################################
set(CLOUD_WALKER_FILES ${UTIL_FILES}
        cloud_walker15/cloud_walker.cpp cloud_walker15/cloud_walker.h)
add_executable(CloudWalker ${CLOUD_WALKER_FILES} cloud_walker15/main_cloud_walker.cpp)
InitBoostHashSFMTEigen(CloudWalker)

#### CloudWalkerAP
add_executable(CloudWalkerAP ${CLOUD_WALKER_FILES} cloud_walker15/main_cloud_ap.cpp ${GROUND_TRUTH_FILES})
InitBoostHashSFMTEigen(CloudWalkerAP)

target_compile_definitions(CloudWalkerAP PRIVATE -DGROUND_TRUTH=1)

#### cloud-walker-bench
add_executable(CloudWalkerBench ${CLOUD_WALKER_FILES} cloud_walker15/main_cloud_ap.cpp ${GROUND_TRUTH_FILES})
InitBoostHashSFMTEigen(CloudWalkerBench)

#### cloudwalker(random pairs)
add_executable(cw-rand-gen ${CLOUD_WALKER_FILES} cloud_walker15/main_cloud_ap_rand_pairs.cpp ${GROUND_TRUTH_FILES} ground_truth/random_pair_generator.cpp)
InitBoostHashSFMTEigen(cw-rand-gen)

add_executable(cw-rand-gen-gt ${CLOUD_WALKER_FILES} cloud_walker15/main_cloud_ap_rand_pairs.cpp ${GROUND_TRUTH_FILES} ground_truth/random_pair_generator.cpp)
InitBoostHashSFMTEigen(cw-rand-gen-gt)
target_compile_definitions(cw-rand-gen-gt PRIVATE -DGROUND_TRUTH=1)

############ playground
add_subdirectory(playground)