cmake_minimum_required(VERSION 3.6)
project(APS)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost REQUIRED serialization program_options)
set(SOURCE_FILES
        stat.cpp stat.h
        file_serialization.h graph.cpp graph.h node_pair.h
        bprw.cpp bprw.h local_push.cpp local_push.h
        cloud_walker.cpp cloud_walker.h
        dfg.cpp dfg.h
        linearD.cpp linearD.h
        link_cut.cpp link_cut.h
        simrank.cpp simrank.h
        sling.cpp sling.h sling_graph.h ../SFMT/SFMT.c
        main.cpp)

# not compiled yet : tkde17_sim.cpp tkde17_sim.h, reads

add_executable(APS ${SOURCE_FILES})
target_include_directories(APS PRIVATE ../sparsehash-yche/include/)
target_include_directories(APS PRIVATE ../sparsepp)
target_include_directories(APS PRIVATE ../eigen)
target_include_directories(APS PRIVATE ../SFMT)
if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
    target_include_directories(APS PRIVATE ${Boost_INCLUDE_DIRS})
endif ()
target_compile_options(APS PRIVATE -O3)
# target_link_libraries(APS PRIVATE boost_serialization boost_program_options)
target_link_libraries(APS PRIVATE ${Boost_LIBRARIES})

# ground truth query
add_executable(ground_truth main_ground_truth.cpp graph.cpp simrank.cpp)
target_include_directories(ground_truth PRIVATE ../eigen)
target_include_directories(ground_truth PRIVATE ../sparsepp)
target_include_directories(ground_truth PRIVATE ../sparsehash-yche/include/)

if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
    target_include_directories(ground_truth PRIVATE ${Boost_INCLUDE_DIRS})
endif ()
target_compile_options(ground_truth PRIVATE -O3)
target_link_libraries(ground_truth PRIVATE ${Boost_LIBRARIES})