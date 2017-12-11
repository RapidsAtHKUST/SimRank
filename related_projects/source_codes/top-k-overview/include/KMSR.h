#ifndef _KMSR_H_
#define _KMSR_H_

#include "config.h"
#include "simrankmethod.h"

class KMSR : public SimRankMethod {
public:
    KMSR(int maxsteps, int sampleNum, double decayfactor, int *graph_src, int *graph_dst, int mvid, char *graph);

    ~KMSR() {}

    virtual void run(int qv, int k);

    void initialize() {}

private:
    void save(vector<SimRankValue> &res, int k) {
        sort(res.begin(), res.end());
        results.clear();
        for (int i = 0; i < k; ++i) {
            results.push_back(res[i]);
        }
    }

    double calSimRank(int queryv, int otherv);

private:
    int *graphSrc;
    int *graphDst;
    int maxVertexId;

    char graphName[125];
    Random randomGenerator;
};

#endif /*_KMSR_H_*/
