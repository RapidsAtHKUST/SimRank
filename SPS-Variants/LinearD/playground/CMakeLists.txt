add_executable(test_serialization main_test_egien_serialization.cpp boost_serialization_eigen.h)
target_include_directories(test_serialization PRIVATE ../../../dependencies/include)
target_include_directories(test_serialization PRIVATE ../../../dependencies/sparsehash-yche/include/)
target_include_directories(test_serialization PRIVATE ../../../dependencies/sparsepp)

target_link_libraries(test_serialization ${Boost_LIBRARIES})