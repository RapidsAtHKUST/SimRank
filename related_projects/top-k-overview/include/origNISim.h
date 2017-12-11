/*************************************************************************
    > File Name: KronSimRank.h
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Fri 20 Mar 2015 11:40:11 PM CST
 ************************************************************************/
#ifndef __ORIGNISIM__
#define __ORIGNISIM__

#include <fstream>
#include <math.h>
#include <armadillo>
#include "simrankmethod.h"

using namespace arma;

class origNISim : public SimRankMethod {
public:
    origNISim(double decayfactor, int *orig_graph_src, int *orig_graph_dst, int rank, int mvid, bool isinit,
              char *inpath) :
            SimRankMethod(0, 0, decayfactor),
            origGraphSrc(orig_graph_src), origGraphDst(orig_graph_dst), maxVertexId(mvid), Rank(rank), isInit(isinit) {
        strcpy(graphName, inpath);
        sprintf(Kupath, "dataset/%s/index/nisim/OrigKron/%s.Ku", graphName, graphName);
        sprintf(Kvpath, "dataset/%s/index/nisim/OrigKron/%s.Kv", graphName, graphName);
        sprintf(V_rpath, "dataset/%s/index/nisim/OrigKron/%s.V_r", graphName, graphName);
        sprintf(Apath, "dataset/%s/index/nisim/OrigKron/%s.A", graphName, graphName);
    }

    ~origNISim() {};

    void initialize();

    void run(int qv, int k);

    SimRankValue getRes(int idx) {
        return results[idx];
    }

    double getScore(int i, int j);
//......save method 
private:
    void save(vector<SimRankValue> &res, int k) {
        sort(res.begin(), res.end());
        results.clear();
        for (int i = 0; i < k; i++) {
            results.push_back(res[i]);
        }

    }

private:
    Mat<double> A;
    Mat<double> Ku;
    Mat<double> Kv;
    Mat<double> V_r;
    int Rank;
    char graphName[125];
    //the following is not needed.
    int *origGraphSrc;
    int *origGraphDst;
    bool isInit;
    int maxVertexId;

    char Kupath[125];
    char V_rpath[125];
    char Apath[125];
    char Kvpath[125];


};

#endif
