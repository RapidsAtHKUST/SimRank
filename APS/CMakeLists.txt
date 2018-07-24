cmake_minimum_required(VERSION 3.6)
project(APS)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost REQUIRED serialization program_options)

# not compiled yet : reads
set(SOURCE_FILES
        stat.cpp stat.h file_serialization.h graph.cpp graph.h node_pair.h
        bprw.cpp bprw.h local_push.cpp local_push.h
        cloud_walker.cpp cloud_walker.h
        dfg.cpp dfg.h
        linearD.cpp linearD.h
        link_cut.cpp link_cut.h
        tkde17_sim.cpp tkde17_sim.h
        simrank.cpp simrank.h
        sling.cpp sling.h sling_graph.h ../dependencies/SFMT/SFMT.c
        main.cpp)

set(DYNAMIC_SOURCE_FILES
        stat.cpp stat.h file_serialization.h graph.cpp graph.h node_pair.h
        bprw.cpp bprw.h
        simrank.cpp simrank.h
        main_dynamic_exp.cpp)

set(SFMT_COMPILE_FLAGS -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)

macro(InitBoostHashSFMTEigen exec_name)
    target_include_directories(${exec_name} PRIVATE ${Boost_INCLUDE_DIRS})
    target_include_directories(${exec_name} PRIVATE ../dependencies/sparsehash-yche/include)
    target_include_directories(${exec_name} PRIVATE ../dependencies/sparsepp)
    target_include_directories(${exec_name} PRIVATE ../dependencies/SFMT)
    target_include_directories(${exec_name} PRIVATE ../dependencies/include)
endmacro()


############################### release: APS #####################################################
add_executable(APS ${SOURCE_FILES})
InitBoostHashSFMTEigen(APS)

if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
    target_include_directories(APS PRIVATE ${Boost_INCLUDE_DIRS})
endif ()
target_compile_options(APS PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
target_link_libraries(APS PRIVATE ${Boost_LIBRARIES})

################### dynamic exp
add_executable(dynamic-flp ${DYNAMIC_SOURCE_FILES} local_push.cpp local_push.h)
InitBoostHashSFMTEigen(dynamic-flp)
target_link_libraries(dynamic-flp PRIVATE ${Boost_LIBRARIES})
target_compile_options(dynamic-flp PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
target_compile_definitions(dynamic-flp PRIVATE -DFLP=1)

############# dynamic-rlp
add_executable(dynamic-rlp ${DYNAMIC_SOURCE_FILES} local_push.cpp local_push.h)
InitBoostHashSFMTEigen(dynamic-rlp)
target_link_libraries(dynamic-rlp PRIVATE ${Boost_LIBRARIES})
target_compile_options(dynamic-rlp PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})

###### dynamic tsf
add_executable(dynamic-tsf graph.cpp main_dynamic_tsf_exp.cpp)
target_include_directories(dynamic-tsf PRIVATE ${Boost_INCLUDE_DIRS})
target_compile_options(dynamic-tsf PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
InitBoostHashSFMTEigen(dynamic-tsf)

############################ playground #########################################3
add_executable(ground_truth main_ground_truth.cpp graph.cpp simrank.cpp)
InitBoostHashSFMTEigen(ground_truth)
target_compile_options(ground_truth PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
target_link_libraries(ground_truth PRIVATE ${Boost_LIBRARIES})