add_executable(serialization_test serialization_test.cpp)
target_include_directories(serialization_test PRIVATE ../../../dependencies/sparsepp)
target_include_directories(serialization_test PRIVATE ${Boost_INCLUDE_DIRS})

add_executable(playground playground.cpp)