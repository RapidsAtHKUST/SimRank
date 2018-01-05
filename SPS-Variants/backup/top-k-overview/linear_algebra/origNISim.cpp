/*
 * input is the normalized adjacency matrix W_t
 * output is the similarity matrix
 * the matrix V in this method is V_T traditionally
 */
#include "origNISim.h"

void origNISim::run(int qv, int k) {
    if (isInit == false) {//generate U,A,Vr
        isInit = true;
        firstRun = false;
        initialize();
    } else {//read Ku,Kv,A,Vr from file
        if (firstRun == true) {
            A.load(Apath);
            V_r.load(V_rpath);
            Ku.load(Kupath);
            Kv.load(Kvpath);
        }
    }
    double score = 0.0;
    vector<SimRankValue> res;
    res.resize(maxVertexId);
    for (int i = 0; i < maxVertexId; i++) {
        score = getScore(qv, i);
        res[i].setVid(i);
        res[i].setValue(score);
    }
    save(res, k);
}

void origNISim::initialize() {
    mat W_t(maxVertexId, maxVertexId, fill::zeros);
    for (int i = 0; i < maxVertexId; i++) {
        int e = origGraphSrc[i + 1], s = origGraphSrc[i];
        for (int j = s; j < e; j++) {
            W_t(i, origGraphDst[j]) = 1.0;
        }
    }
    double sumRow;
    double Degr = 0;
    for (int i = 0; i < maxVertexId; i++) {
        sumRow = 0;
        for (int j = 0; j < maxVertexId; j++)
            sumRow += W_t(i, j);
        if (sumRow == 0.0) {
            printf("node %d has no ingoing edges\n", i);
        } else {
            for (int j = 0; j < maxVertexId; j++)
                W_t(i, j) = W_t(i, j) / sumRow;
        }
        Degr += sumRow;
    }
    //start svd
    Mat<double> U;
    Col<double> s;
    Mat<double> V_t;
    svd(U, s, V_t, W_t);
    mat V = V_t.t();

    U = U.submat(0, 0, maxVertexId - 1, Rank - 1);
    V = V.submat(0, 0, Rank - 1, maxVertexId - 1);
    s = s.submat(0, 0, Rank - 1, 0);
    Ku = kron(U, U);//kronecker roduct
    mat sigma = kron(s, s);//one column
    mat K_sigma = diagmat(sigma);
    Kv = kron(V, V);

    mat K_vu = Kv * Ku;

    mat I(maxVertexId, maxVertexId);
    I.eye();
    A = inv(inv(K_sigma) - decayFactor * K_vu);
    V_r = Kv * vectorise(I);

    A.save(Apath);
    V_r.save(V_rpath);
    Kv.save(Kvpath);
    Ku.save(Kupath);
}


double origNISim::getScore(int i, int j) {//if i==j, then return 0;
    mat V_l = (Ku.row(i * maxVertexId + j)) * A;
    Mat<double> temp;
    if (i == j)
        return 0;
    else {
        temp = (1 - decayFactor) * (0 + decayFactor * V_l * V_r);
        return temp(0, 0);
    }
}

