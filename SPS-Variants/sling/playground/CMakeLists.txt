add_executable(serialization_test serialization_test.cpp)
target_include_directories(serialization_test PRIVATE ../../../sparsepp)
target_include_directories(serialization_test PRIVATE ${Boost_INCLUDE_DIRS})