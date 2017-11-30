#ifndef _KMSRINDEX_H_
#define _KMSRINDEX_H_

#include "config.h"
#include "simrankmethod.h"

class KMSRIndex : public SimRankMethod {
public:
    KMSRIndex(int maxsteps, int sampleNum, double decayfactor, int *graph_src, int *graph_dst, int mvid,
              char *graph, bool isinit,
              int *ori_graph_src, int *ori_graph_dst);

    ~KMSRIndex() {
        if (sampleBipartite != NULL) {
            for (int i = 0; i < maxVertexId; ++i) {
                delete[] sampleBipartite[i];
            }
            delete[] sampleBipartite;
        }

        for (int i = 0; i < T; ++i) {
            delete[] ga_mma[i];
        }
        delete[] ga_mma;
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

    double calSimRank(int queryv, int otherv);

    /* API for proprecess */
    bool preprocess();

    void computeGamma();

    int performRandomWalk(int vertex, int stepLen, int *route);

    int findIntersection(int qv, int othervs);

    void expandQueueOnUndirected(int vertex, queue<int> &Que, bool *vis, int *dist = NULL);

    void compDist(int vertex, int *dist, bool *vis);

    double computeBeta(int vertex, int d, int *dist);

private:
    int *graphSrc;
    int *graphDst;
    int maxVertexId;
    bool isInit;

    char graphName[125];
    Random randomGenerator;

    /* index related field */
    int **sampleBipartite;
    int P, T, Q, R, R1, R2;
    double epsilon;
    double **ga_mma;
    int *oriGraphSrc;
    int *oriGraphDst;
};

#endif /*_SimRankMOD14_H_*/
