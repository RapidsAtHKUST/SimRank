# 10 Algorithms for Top-K

Zhipeng Zhang, vldb2017

## Iterative

algorithm | files
--- | ---
02kdd, naive | [naiveSR.h](./iterative/naiveSR.h), [naiveSR.cpp](./iterative/naiveSR.cpp)
08vldb, partial sum | [PartialSR.h](iterative/PartialSR.h), [PartialSR.cpp](iterative/PartialSR.cpp)
13icde, similar to 08vldb | [OIPSimRank.h](iterative/OIPSimRank.h), [OIPSimRank.cpp](iterative/OIPSimRank.cpp)

## Linear Algebra

algorithm | files
--- | ---
10edbt, ni-sim |[origNISim.h](linear_algebra/origNISim.h), [origNISim.cpp](linear_algebra/origNISim.cpp), [NISim.h](linear_algebra/NISim.h), [NISim.cpp](linear_algebra/NISim.cpp)
13icde, simMat | [SimMat.h](linear_algebra/SimMat.h), [SimMatEVD.h](linear_algebra/SimMatEVD.h), [SimMatEVD.cpp](linear_algebra/SimMatEVD.cpp), [SimMatSVD.h](linear_algebra/SimMatSVD.h), [SimMatSVD.cpp](linear_algebra/SimMatSVD.cpp)
15vldb, Par-sim | [ParSRSimRank.h](linear_algebra/ParSRSimRank.h), [ParSRSimRank.cpp](linear_algebra/ParSRSimRank.cpp)

## Random Walk

algorithm | files
--- | ---
05www, fpsr(mc) | [FPSR.h](random_walk/FPSR.h), [FPSR.cpp](random_walk/FPSR.cpp), [FingerPrint.h](random_walk/FingerPrint.h), [FingerPrint.cpp](random_walk/FingerPrint.cpp)
12icde, top-sim | [topsim.h](random_walk/topsim.h), [topsim.cpp](random_walk/topsim.cpp), [simmap.hpp](random_walk/simmap.hpp)
14sigmod, KM-SR | [KMSR.h](random_walk/KMSR.h), [KMSR.cpp](random_walk/KMSR.cpp), [KMSRIndex.h](random_walk/KMSRIndex.h), [KMSRIndex.cpp](random_walk/KMSRIndex.cpp)
15vldb, tsf(shao) | [TSF.h](tsf/TSF.h), [TSF.cpp](tsf/TSF.cpp), [gsinterface.h](tsf/gsinterface.h), [gsmanager.hpp](tsf/gsmanager.hpp), [rgsmanager.hpp](tsf/rgsmanager.hpp), [rsamplegraph.hpp](tsf/rsamplegraph.hpp)

