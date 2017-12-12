#ifndef _FPSR_H_
#define _FPSR_H_

#include "../config.h"
#include "../simrankmethod.h"
#include "FingerPrint.h"

class FPSR : public SimRankMethod {
public:
    FPSR(int maxSteps, int sampleNum, double decayFactor, int *graph_src, int *graph_dst, int mvid, char *graph,
         bool isinit);

    ~FPSR() {
        for (int i = 0; i < sampleNum; ++i) {
            delete index[i];
        }
        delete[] index;
    }

    virtual void run(int qv, int k);

    void initialize();

private:
    void save(vector<SimRankValue> &res, int k) {
        sort(res.begin(), res.end());
        results.clear();
        for (int i = 0; i < k; ++i) {
            results.push_back(res[i]);
        }
    }

    void calSimRank(int queryv, vector<SimRankValue> &);

    void indexBuilder(int sn);

private:
    int *graphSrc;
    int *graphDst;
    int maxVertexId;
    bool isInit;
    bool isLoad;
    char graphName[125];
    FingerPrint **index;
    Random randomGenerator;
};

#endif 
