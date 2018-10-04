add_executable(play1 test_graph_io.cpp ../util/graph_yche.cpp ../util/pretty_print.h ../util/log.cpp)
target_compile_options(play1 PRIVATE -O3 -g)

add_executable(play_stl test_stl.cpp)