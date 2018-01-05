/*************************************************************************
    > File Name: EffiSimRank.h
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Sun 22 Mar 2015 11:04:11 AM CST
 ************************************************************************/
#ifndef __SIMMAT_H__
#define __SIMMAT_H__

#include <complex>
#include <math.h>
#include <fstream>
#include <armadillo>
#include "../simrankmethod.h"

using namespace arma;

class SimMat : public SimRankMethod {
public:
    SimMat(double decayfactor, int *orig_graph_src, int *orig_graph_dst, int rank, int mvid, bool isinit, char *inpath)
            : SimRankMethod(0, 0, decayfactor), origGraphSrc(orig_graph_src), origGraphDst(orig_graph_dst),
              maxVertexId(mvid), Rank(rank), isInit(isinit) {
        strcpy(graphName, inpath);
    }

    virtual ~SimMat() {}

    virtual void run(int qv, int k)=0;

    virtual void initialize()=0;

    SimRankValue getRes(int idx) {
        return results[idx];
    }

protected:
    void save(vector<SimRankValue> &res, int k) {
        sort(res.begin(), res.end());
        results.clear();
        for (int i = 0; i < k; i++) {
            results.push_back(res[i]);
        }
    }

    double getSu(int i, int j) {
        if (i == j)
            return 0;
        double temp = sqrt(norm(l_(i) * r_(j)));
        return (l_dev(i) * r_dev(j) + temp) * maxVertexId;
    }

    double getScore(int i, int j) {
        if (i == j)
            return 0;
        cx_mat x = L.row(i) * R.col(j);
        double a = imag(x(0, 0));
        double b = real(x(0, 0));
        return sqrt(a * a + b * b);
    }

protected:
    Mat<cx_double> L;
    Mat<cx_double> R;
    Col<cx_double> l_;
    Row<cx_double> r_;
    Col<double> l_dev;
    Row<double> r_dev;

    char graphName[125];
    int Rank;
    priority_queue<pair<double, int>, vector<pair<double, int> >, greater<pair<double, int> > > coll;
    //following is not necessary
    int *origGraphSrc;
    int *origGraphDst;
    bool isInit;
    int maxVertexId;

    char Lpath[125];
    char Rpath[125];
    char l_path[125];
    char r_path[125];
    char l_devpath[125];
    char r_devpath[125];

};

#endif
