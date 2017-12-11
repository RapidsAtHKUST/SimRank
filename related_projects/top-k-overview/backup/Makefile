# Build targets
SimRank_O= core/SRBenchmark.cpp \
           core/TSF.cpp \
		   core/PartialSR.cpp \
		   core/FingerPrint.cpp \
		   core/KMSR.cpp \
		   core/KMSRIndex.cpp \
		   core/FPSR.cpp \
		   core/topsim.cpp \
		   core/mytime.cpp \
		   core/NISim.cpp\
		   core/origNISim.cpp\
		   core/SimMatEVD.cpp\
		   core/SimMatSVD.cpp\
		   core/OptSimMatSVD.cpp\
		   core/OIPSimRank.cpp\
		   core/ParSRSimRank.cpp\
           core/naiveSR.cpp\
	
USER_FLAGS=-Wno-unused-result -Wno-unused-but-set-variable -Wno-sign-compare
USER_LIBS=

# Compiler flags
CC  = gcc
CXX = g++
CFLAGS=$(USER_FLAGS) -O3 -fPIC -Wall -g  -I ./include -I ./include/arma -l lapack -l blas 
CXXFLAGS=$(CFLAGS)
LDFLAGS=-lpthread

# The name of the excution that will be built
SimRank=SRbenchMark

all:
	$(CXX) -w  -pthread $(LDFLAGS) -o $(SimRank) $(SimRank_O) $(CXXFLAGS)
	
clean:
	rm -f $(SimRank)
	find . -name '*.o' -print | xargs rm -f
