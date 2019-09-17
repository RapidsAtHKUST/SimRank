include_directories(${CMAKE_SOURCE_DIR}})
add_executable(play1 test_graph_io.cpp ../util/pretty_print.h ../util/log.cpp ../util/graph_yche.cpp)
target_compile_options(play1 PRIVATE -O3 -g)

add_executable(play_stl test_stl.cpp)