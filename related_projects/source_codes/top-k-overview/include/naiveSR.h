#ifndef _NAIVESR_H_
#define _NAIVESR_H_

#include "config.h"
#include "simrankmethod.h"

class NaiveSR : public SimRankMethod {
public:
    NaiveSR(int maxsteps, double decayfactor, int *graph_src, int *graph_dst, int mvid, char *graph,
            bool isinit = false, bool ishalf = true) :
            SimRankMethod(maxsteps, 0, decayfactor),
            graphSrc(graph_src), graphDst(graph_dst), maxVertexId(mvid), isInit(isinit), isHalf(ishalf) {
        strcpy(graphName, graph);
        srvalue[0] = new double *[mvid];
        srvalue[1] = new double *[mvid];
        for (int i = 0; i < mvid; ++i) {
            srvalue[0][i] = new double[mvid];
            srvalue[1][i] = new double[mvid];
            memset(srvalue[0][i], 0, sizeof(double) * (mvid));
            memset(srvalue[1][i], 0, sizeof(double) * (mvid));
        }
        for (int i = 0; i < mvid; ++i) {
            srvalue[0][i][i] = 1.0;
            srvalue[1][i][i] = 1.0;
        }
    }

    ~NaiveSR() {
        for (int i = 0; i < maxVertexId; ++i) {
            delete[] srvalue[0][i];
            delete[] srvalue[1][i];
        }
        delete[] srvalue[0];
        delete[] srvalue[1];
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

private:
    int *graphSrc;
    int *graphDst;
    int maxVertexId;
    bool isInit;
    char graphName[125];
    double **srvalue[2];
    bool isHalf;
};

#endif
