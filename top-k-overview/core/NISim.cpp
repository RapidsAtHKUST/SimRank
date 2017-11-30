/*************************************************************************
    > File Name: KroSimRank.cpp
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Tue 10 Mar 2015 05:58:41 PM CST
 ************************************************************************/
/*
 * input is the normalized adjacency matrix W_t
 * output is the similarity matrix
 * the matrix V in this method is V_T traditionally
 */
#include "NISim.h"

void NISim::run(int qv, int k) {
    if (isInit == false) {//generate U,A,Vr
        isInit = true;
        firstRun = false;
        initialize();
    } else {//read U,A,Vr from file
        if (firstRun == true) {
            A.load(Apath);
            V_r.load(V_rpath);
            U.load(Upath);
        }
    }
    double score = 0.0;
    vector<SimRankValue> res;
    res.resize(maxVertexId);
    for (int i = 0; i < maxVertexId; i++) {
        if (qv == i) {
            score = 0;//the topk cannot be itself
        } else {
            score = getScore(qv, i);
        }
        res[i].setVid(i);
        res[i].setValue(score);
    }
    save(res, k);
    //compute simrank value for(S(qv,other))

}

void NISim::initialize() {
    mat W_t(maxVertexId, maxVertexId, fill::zeros);
    for (int i = 0; i < maxVertexId; i++) {
        int e = origGraphSrc[i + 1], s = origGraphSrc[i];
        for (int j = s; j < e; j++) {
            W_t(i, origGraphDst[j]) = 1.0;
        }
    }
    W_t = W_t.t();
    double sumRow;
    double Degr = 0;
    vector<double> d;
    d.resize(maxVertexId);
    for (int i = 0; i < maxVertexId; i++) {
        sumRow = 0;
        for (int j = 0; j < maxVertexId; j++)
            sumRow += W_t(i, j);
        if (sumRow < 0.00000000001) {
            d[i] = 1;
        } else {
            for (int j = 0; j < maxVertexId; j++) {
                W_t(i, j) = W_t(i, j) / sumRow;
                d[i] = (sumRow - 1.0) / sumRow;

            }
        }
        Degr += sumRow;
    }
    //start SVD for W_t
    Col<double> s;
    Mat<double> V_t;
    svd(U, s, V_t, W_t);//finish at 15min
    mat V = V_t.t();
    U = U.submat(0, 0, maxVertexId - 1, Rank - 1);
    V = V.submat(0, 0, Rank - 1, maxVertexId - 1);
    mat vu = V * U;
    mat K_vu = kron(vu, vu);
    s = s.submat(0, 0, Rank - 1, 0);
    mat sigma = kron(s, s);//one column
    mat sigma_1 = 1. / sigma;
    mat K_sigma_1 = diagmat(sigma_1);
    mat I(maxVertexId, maxVertexId);
    I.eye();
    A = inv(K_sigma_1 - decayFactor * K_vu);

    V_r = d[0] * kron(V.col(0), V.col(0));
    for (int i = 1; i < maxVertexId; i++) {
        V_r = V_r + d[i] * kron(V.col(i), V.col(i));
    }
    A.save(Apath);
    V_r.save(V_rpath);
    U.save(Upath);
}

double NISim::getScore(int i, int j) {
    mat V_l = kron(U.row(i), U.row(j)) * A;
    Mat<double> temp;
    if (i == j)
        return 0;
    else {
        temp = decayFactor * V_l * V_r;
        return temp(0, 0);
    }
}

