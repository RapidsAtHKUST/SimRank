add_executable(bin_converter edge_lst_to_bin_converter.cpp graph_yche.cpp)
target_compile_options(bin_converter PRIVATE -O3 -g)



# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
find_package(Boost REQUIRED program_options)
if (Boost_FOUND)
    message(${Boost_INCLUDE_DIRS})
endif ()

aux_source_directory(. DIR_LIB_SRCS)
# 生成链接库
add_library (Utils ${DIR_LIB_SRCS} ../../dependencies/SFMT/SFMT.c)

set(exec_name Utils)

target_include_directories(${exec_name} PRIVATE ../)
target_include_directories(${exec_name} PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(${exec_name} PRIVATE ../../dependencies/sparsehash-yche/include)
target_include_directories(${exec_name} PRIVATE ../../dependencies/sparsepp)
target_include_directories(${exec_name} PRIVATE ../../dependencies/SFMT)
target_include_directories(${exec_name} PRIVATE ../../dependencies/include)

set(SFMT_COMPILE_FLAGS -DSFMT_MEXP=607 -msse2 -DHAVE_SSE2)

target_compile_definitions(${exec_name} PRIVATE -DHAS_OPENMP=1 -DSFMT=1)
target_compile_options(${exec_name} PRIVATE -O3 -g ${SFMT_COMPILE_FLAGS} -march=native)

