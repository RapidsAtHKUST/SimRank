add_executable(play0 playground.cpp)
target_include_directories(play0 PRIVATE ../../../dependencies/sparsepp)
target_include_directories(play0 PRIVATE ${Boost_INCLUDE_DIRS})