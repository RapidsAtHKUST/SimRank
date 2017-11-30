cmake_minimum_required(VERSION 3.6)
project(SRBenchMark)

set(CMAKE_CXX_STANDARD 11)

# include the whole file
aux_source_directory(./core DIR_SRCS)
set(OTHER_FILES include/config.h include/FingerPrint.h include/FPSR.h include/gsinterface.h include/gsmanager.hpp include/KMSR.h include/KMSRIndex.h
        include/meminfo.hpp include/mymkdir.h include/myqueue.hpp include/mytime.h include/naiveSR.h include/NISim.h include/OIPSimRank.h
        include/OptSimMatSVD.h include/origNISim.h include/ParSRSimRank.h include/PartialSR.h include/random.h
        include/rgsmanager.hpp include/rsamplegraph.hpp include/simmap.hpp include/SimMat.h include/SimMatEVD.h include/SimMatSVD.h
        include/simrankmethod.h include/simrankvalue.hpp include/simrankvalue.hpp include/topsim.h include/TSF.h)

# add the executable files
add_executable(SRBenchMark ${DIR_SRCS} ${OTHER_FILES})

# linear algebra, hash table
target_compile_options(SRBenchMark PRIVATE -O3)
target_include_directories(SRBenchMark PRIVATE ../armadillo-code/include)
target_include_directories(SRBenchMark PRIVATE ../sparsehash-c11)
target_include_directories(SRBenchMark PRIVATE include)
target_link_libraries(SRBenchMark lapack blas pthread)