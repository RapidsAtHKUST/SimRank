#ifndef _OIPSIMRANK_H_
#define _OIPSIMRANK_H_

#include "config.h"
#include "simrankmethod.h"


class OIPSimRank : public SimRankMethod {
public:
    OIPSimRank(int maxsteps, double decayfactor, int *graph_src, int *graph_dst, int mvid, char *graph,
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
        mstSrc = new int[mvid];//因为mst中加入了节点‘#’,因此edge的数目为mvid
        memset(mstSrc, -1, sizeof(int) * (mvid));
        mstDst = new int[mvid];
        memset(mstDst, -1, sizeof(int) * (mvid));
        neighbors = new int *[mvid];
        for (int i = 0; i < mvid; i++) {
            neighbors[i] = new int[1000];
            memset(neighbors[i], 0, sizeof(int) * 1000);
        }
    }

    ~OIPSimRank() {
        for (int i = 0; i < maxVertexId; ++i) {
            delete[] srvalue[0][i];
            delete[] srvalue[1][i];
            delete[] neighbors[i];
        }
        delete[]neighbors;
        delete[] srvalue[0];
        delete[] srvalue[1];
        delete[]mstSrc;
        delete[]mstDst;
    }

    virtual void run(int qv, int k);

    void initialize();//given graph G, generate all simrank scores
    void
    generateWeightMatrix_MST();//given graph G,generate weight matrix, then given weight matrix ,
    // generate mst store in mstDst,mstSrc,mstWeight
    void OP(int u, int iter, double *pSum);//for each vertex y,given partialsum (u,y), return S(u,y)
    int getIntersection(int a, int b);

private:
    void save(vector<SimRankValue> &res, int k) {
        sort(res.begin(), res.end());
        results.clear();
        for (int i = 0; i < k; ++i) {
            results.push_back(res[i]);
        }
    }

private:
//	to store reversed graph
    int *graphSrc;
    int *graphDst;
    int maxVertexId;
    bool isInit;
    char graphName[125];
//	to store all-pair simrank values
    double **srvalue[2];
//	to store mst
    int *mstSrc;
    int *mstDst;
    int *mstWeight;
    int **neighbors;
    bool isHalf;
};

#endif /* _OIPSIMRANK_H_ */

