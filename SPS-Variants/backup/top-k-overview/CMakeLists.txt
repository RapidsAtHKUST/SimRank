cmake_minimum_required(VERSION 3.6)
project(SRBenchMark)

set(CMAKE_CXX_STANDARD 11)

# include the whole file
aux_source_directory(. DIR_SRCS)
aux_source_directory(./iterative DIR_SRCS)
aux_source_directory(./random_walk DIR_SRCS)
aux_source_directory(./linear_algebra DIR_SRCS)
aux_source_directory(./tsf DIR_SRCS)
set(OTHER_FILES config.h random_walk/FingerPrint.h random_walk/FPSR.h tsf/gsinterface.h tsf/gsmanager.hpp random_walk/KMSR.h random_walk/KMSRIndex.h
        meminfo.hpp mymkdir.h myqueue.hpp mytime.h iterative/naiveSR.h linear_algebra/NISim.h iterative/OIPSimRank.h
        linear_algebra/OptSimMatSVD.h linear_algebra/origNISim.h linear_algebra/ParSRSimRank.h iterative/PartialSR.h random.h
        tsf/rgsmanager.hpp tsf/rsamplegraph.hpp random_walk/simmap.hpp linear_algebra/SimMat.h linear_algebra/SimMatEVD.h linear_algebra/SimMatSVD.h
        simrankmethod.h simrankvalue.hpp simrankvalue.hpp random_walk/topsim.h tsf/TSF.h)

# add the executable files
add_executable(SRBenchMark ${DIR_SRCS} ${OTHER_FILES})

# linear algebra, hash table
target_compile_options(SRBenchMark PRIVATE -O3)
target_include_directories(SRBenchMark PRIVATE ../../armadillo-code/include)
target_include_directories(SRBenchMark PRIVATE ../../sparsehash-yche/include/)
target_include_directories(SRBenchMark PRIVATE include)
target_link_libraries(SRBenchMark lapack blas pthread)
