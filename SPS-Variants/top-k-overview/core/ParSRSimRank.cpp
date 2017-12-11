#include "ParSRSimRank.h"

void ParSRSimRank::run(int qv, int k) {
    if (type == 0)
        Par_SR(qv, k);
    else if (type == 1)
        PrunPar_SR(qv, k);
    else {
        printf("invalid method for PSR, valid ones are 0, 1.\n");
        return;
    }
}

void ParSRSimRank::Par_SR(int qv, int topk) {
    int k = maxSteps;
    double **u = new double *[k + 1];
    for (int i = 0; i < k + 1; i++) {
        u[i] = new double[maxVertexId];
    }

    double **v = new double *[2];
    v[0] = new double[maxVertexId];
    v[1] = new double[maxVertexId];

    vector<double> d;
    d.resize(maxVertexId);
    for (int nodeId = 0; nodeId < maxVertexId; nodeId++) {
        int os = orig_graphSrc[nodeId];
        int oe = orig_graphSrc[nodeId + 1];
        int xDeg = oe - os;
        if (xDeg == 0)
            d[nodeId] = 1;
        else
            d[nodeId] = 1.0 * (xDeg - 1) / xDeg;
    }
    //method body
    for (int i = 0; i < k + 1; i++)
        memset(u[i], 0, sizeof(double) * maxVertexId);
    //initialize u[l]
    u[0][qv] = 1;
    for (int l = 1; l < k + 1; l++) {
        //compute u[l] using u[l-1]
        for (int i = 0; i < maxVertexId; i++) {
            //compute u[l][i]
            int os = orig_graphSrc[i], oe = orig_graphSrc[i + 1];
            for (int x = os; x < oe; x++) {
                int temp = orig_graphDst[x];
                int xDeg = graphSrc[temp + 1] - graphSrc[temp];
                u[l][i] += 1.0 / xDeg * u[l - 1][temp];
            }
        }
    }
    //use u[l] to compute v[k]
    for (int i = 0; i < maxVertexId; i++)
        v[0][i] = d[i] * u[k][i];

    for (int l = 1; l < k + 1; l++) {
        //use v[l-1] to compute v[l], use v[1-(l&1)] to compute v[l&1]
        //memset(v[l&1],0,sizeof(double)*maxVertexId);
        for (int i = 0; i < maxVertexId; i++) {
            v[l & 1][i] = d[i] * u[k - l][i];
            int rs = graphSrc[i], re = graphSrc[i + 1];
            int iDeg = re - rs;
            for (int x = rs; x < re; x++) {
                v[l & 1][i] += decayFactor * 1.0 / iDeg * v[1 - (l & 1)][graphDst[x]];
            }
        }

    }

    vector<SimRankValue> res;
    res.resize(maxVertexId);
    for (int i = 0; i < maxVertexId; ++i) {
        res[i].setVid(i);
        res[i].setValue(v[k & 1][i]);
        if (qv == i)// qv is most similar to itself
            res[i].setValue(0);
    }
    save(res, topk);

    for (int i = 0; i < k + 1; i++)
        delete[] u[i];
    delete[] u;
    delete[] v[0];
    delete[] v[1];
    delete[] v;

}

void ParSRSimRank::PrunPar_SR(int qv, int topk) {
    int k = maxSteps;
    double **u = new double *[k + 1];
    for (int i = 0; i < k + 1; i++) {
        u[i] = new double[maxVertexId];
    }

    double **v = new double *[2];
    v[0] = new double[maxVertexId];
    v[1] = new double[maxVertexId];

    //method body   
    for (int i = 0; i < k + 1; i++)
        memset(u[i], 0, sizeof(double) * maxVertexId);
    //initialize u[l]
    u[0][qv] = 1;
    for (int l = 1; l < k + 1; l++) {
        //compute u[l] using u[l-1]
        for (int i = 0; i < maxVertexId; i++) {
            double temp = u[l - 1][i];
            if (temp == 0)
                continue;
            else {
                int rs = graphSrc[i], re = graphSrc[i + 1];
                int deg = re - rs;
                for (int x = rs; x < re; x++) {
                    u[l][graphDst[x]] += temp * 1.0 / deg;
                }
            }
        }
    }
    //use u[l] to compute v[k]
    for (int i = 0; i < maxVertexId; i++)
        v[0][i] = u[k][i];
    for (int l = 1; l < k + 1; l++) {
        //use v[l-1] to compute v[l], use v[1-(l&1)] to compute v[l&1]
        //memset(v[l&1],0,sizeof(double)*maxVertexId);
        for (int i = 0; i < maxVertexId; i++)
            v[l & 1][i] = u[k - l][i];
        for (int i = 0; i < maxVertexId; i++) {
            // v[l&1][i]=u[k-l][i];
            if (v[1 - (l & 1)][i] == 0)
                continue;
            else {
                int os = orig_graphSrc[i], oe = orig_graphSrc[i + 1];
                for (int x = os; x < oe; x++) {
                    int node = orig_graphDst[x];
                    int iDeg = graphSrc[node + 1] - graphSrc[node];
                    v[l & 1][node] += decayFactor * 1.0 / iDeg * v[1 - (l & 1)][i];
                }
            }
        }
    }


    vector<SimRankValue> res;
    res.resize(maxVertexId);
    for (int i = 0; i < maxVertexId; ++i) {
        res[i].setVid(i);
        res[i].setValue((1 - decayFactor) * v[k & 1][i]);
        if (qv == i)
            res[i].setValue(0);
    }
    save(res, topk);

    for (int i = 0; i < k + 1; i++)
        delete[] u[i];
    delete[] u;
    delete[] v[0];
    delete[] v[1];
    delete[] v;
}
