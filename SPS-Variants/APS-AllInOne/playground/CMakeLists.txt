add_executable(play1 test_graph_io.cpp ../util/graph_yche.cpp pretty_print.h)
target_include_directories(play1 PRIVATE ../../SFMT)
target_compile_options(play1 PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})

add_executable(random_pair_gen main_generate_random_pairs.cpp ../util/yche_serialization.h ../util/graph_yche.cpp
        ../util/random_pair_generator.cpp ../util/random_pair_generator.h)
target_include_directories(random_pair_gen PRIVATE ../../SFMT)
target_compile_options(random_pair_gen PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})
target_link_libraries(random_pair_gen ${Boost_LIBRARIES})

add_executable(test_string test_string.cpp)