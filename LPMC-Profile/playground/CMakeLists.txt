add_executable(play1 test_graph_io.cpp ../util/graph_yche.cpp pretty_print.h)
target_include_directories(play1 PRIVATE ../../SFMT)
target_compile_options(play1 PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS})