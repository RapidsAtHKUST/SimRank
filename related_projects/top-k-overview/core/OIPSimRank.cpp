#include "OIPSimRank.h"

void OIPSimRank::run(int qv, int k) {
    if (isInit == false) {
        isInit = true;
        initialize();
    } else {
        char filepath[125];
        sprintf(filepath, "dataset/%s/index/oipdmst/oip.idx", graphName);
        FILE *fp = fopen(filepath, "rb");
        if (fp == NULL) {
            printf("Failed to open the %s file.\n", filepath);
        }
        for (int i = 0; i < maxVertexId; ++i) {
            fread(srvalue[maxSteps & 1][i], sizeof(double), maxVertexId, fp);
        }
        fclose(fp);
    }

    vector<SimRankValue> res;
    res.resize(maxVertexId);
    for (int i = 0; i < maxVertexId; ++i) {
        if (i > qv && srvalue[maxSteps & 1][qv][i] > 0.0) {
            res[i].setVid(i);
            res[i].incValue(srvalue[maxSteps & 1][qv][i]);
        } else if (i < qv && srvalue[maxSteps & 1][i][qv] > 0.0) {
            res[i].setVid(i);
            res[i].incValue(srvalue[maxSteps & 1][i][qv]);
        }
    }
    save(res, k);
}

void OIPSimRank::initialize() {
    //sort the outgoing neighbors for using intersecti
    for (int i = 0; i < maxVertexId; i++)
        sort(graphDst + graphSrc[i], graphDst + graphSrc[i + 1]);
    Time timer;
    timer.start();
    generateWeightMatrix_MST();
    timer.stop();
    printf("time cost for generating MST: %.5lf\n", timer.getElapsedTime());
    int iter = 0;
    double **pSum = new double *[maxVertexId];
    for (int i = 0; i < maxVertexId; i++) {
        pSum[i] = new double[maxVertexId];//pSum[u][y] denotes pSum_Iu^y
    }
    printf("while loop start\n");
    //Time timer;
    timer.start();
    while (iter < maxSteps) {
        for (int i = 0; i < maxVertexId; i++)
            memset(pSum[i], 0, sizeof(double) * maxVertexId);
        for (int i = 0; i < maxVertexId; i++) {
            //each edge in mst
            int v = mstSrc[i];
            int u = mstDst[i];
            if (graphSrc[u + 1] == graphSrc[u])
                continue;
            if (mstSrc[i] == maxVertexId) {// node maxVertexId represents node '#'
                //compute parital[u][y] directly
                for (int y = 0; y < maxVertexId; y++)
                    for (int k = graphSrc[u]; k < graphSrc[u + 1]; k++) {
                        int sidx = y < graphDst[k] ? y : graphDst[k];
                        int didx = y < graphDst[k] ? graphDst[k] : y;
                        pSum[u][y] += srvalue[iter & 1][sidx][didx];

                    }
                OP(u, iter, pSum[u]);
            } else {//given mstSrc[i], compute mstDst[i]
                //use v to compute u
                for (int y = 0; y < maxVertexId; y++) {
                    pSum[u][y] = pSum[v][y];
                    int nid = 0;
                    while (neighbors[i][nid] != 0) {
                        int temp = neighbors[i][nid];
                        nid++;
                        int sidx, didx;
                        if (temp < 0) {
                            temp = -temp - 1;
                            sidx = y < temp ? y : temp;
                            didx = y < temp ? temp : y;
                            pSum[u][y] -= srvalue[iter & 1][sidx][didx];
                        } else {
                            temp = temp - 1;
                            sidx = y < temp ? y : temp;
                            didx = y < temp ? temp : y;
                            pSum[u][y] += srvalue[iter & 1][sidx][didx];
                        }
                    }
                }
                OP(u, iter, pSum[u]);
            }
        }
        iter++;
    }

    timer.stop();
    printf("time cost for while loop of OIP: %.5lf\n", timer.getElapsedTime());
    char filepath[125];
    sprintf(filepath, "dataset/%s/index/oipdmst/oip.idx", graphName);
    FILE *fp = fopen(filepath, "wb");
    for (int i = 0; i < maxVertexId; ++i) {
        fwrite(srvalue[maxSteps & 1][i], sizeof(double), maxVertexId, fp);
    }
    fclose(fp);
    for (int i = 0; i < maxVertexId; i++)
        delete[] pSum[i];
    delete[] pSum;
}

void OIPSimRank::generateWeightMatrix_MST() {
    //output graphSrc,graphDst
    const static int maxWeight = 100000;
    int **A = new int *[maxVertexId + 1];
    for (int i = 0; i < maxVertexId + 1; i++) {
        A[i] = new int[maxVertexId + 1];//include node '#' as node N
        A[i][maxVertexId] = maxWeight;
    }
    //gennerate weight matrix store in A
    for (int i = 0; i < maxVertexId; i++) {
        A[i][i] = 0;
        for (int j = i + 1; j < maxVertexId; j++) {
            int size_i = graphSrc[i + 1] - graphSrc[i];
            int size_j = graphSrc[j + 1] - graphSrc[j];
            int intersection = getIntersection(i, j);
            //if(size_i>size_j)
            A[j][i] = size_i + size_j - 2 * intersection; //given j,compute i
            //else
            A[i][j] = size_i + size_j - 2 * intersection; //given i,compute j
        }
    }
    //node n denote the '#'
    for (int i = 0; i < maxVertexId; i++) {
        int deg = graphSrc[i + 1] - graphSrc[i];
        if (deg != 0)
            deg--;//to compute n elements ,we only need n-1 additions
        A[maxVertexId][i] = deg;
    }

    printf("weight matrix is finished\n");

    //generate mst store in mstSrc, mstDst, mstWeight
    int len = maxVertexId + 1;
    int srcId = maxVertexId;
    int minId = -1;
    int minWeight = 100000;
    bool *isvisited = new bool[len];
    memset(isvisited, 0, sizeof(bool) * len);
    int *lowcost = new int[len];
    int *mst = new int[len];
    // memset(mst, srcId, sizeof(int)*(len));
    for (int i = 0; i < len; i++) {
        mst[i] = srcId;
        lowcost[i] = A[srcId][i];//start from node '#',whose ID is maxVertexId
    }
    isvisited[srcId] = true;
    lowcost[srcId] = 0;
    for (int i = 0; i < len - 1; i++) {
        //choose one edge to insert
        minWeight = 100000;
        for (int j = 0; j < len; j++) {
            if (minWeight > lowcost[j] && isvisited[j] == false) {
                minId = j;
                minWeight = lowcost[j];
            }
        }
        //update isvisited, lowcost,mst
        isvisited[minId] = true;
        mstSrc[i] = mst[minId];
        mstDst[i] = minId;
//		mstWeight[i-1]=A[mstSrc[i-1]][mstDst[i-1]];
        for (int j = 0; j < len; j++) {
            if (lowcost[j] > A[minId][j]) {
                lowcost[j] = A[minId][j];
                mst[j] = minId;
            }
        }
    }

    delete[]lowcost;
    delete[]mst;
    for (int i = 0; i < maxVertexId + 1; i++)
        delete[] A[i];
    delete[] A;
    //initialize neighbors list
    int nid;
    for (int i = 0; i < maxVertexId; i++) {
        int a = mstSrc[i];
        int b = mstDst[i];
        if (a == maxVertexId)//we do not store this, since the god node has no in-neighbors
            continue;
        int as = graphSrc[a], ae = graphSrc[a + 1];
        int bs = graphSrc[b], be = graphSrc[b + 1];
        int nid = 0;
        while (as < ae && bs < be) {
            if (graphDst[as] < graphDst[bs]) {
                neighbors[i][nid] = -(graphDst[as] + 1);
                nid++;
                as++;
            } else if (graphDst[as] == graphDst[bs]) {
                as++;
                bs++;
            } else {
                neighbors[i][nid] = (graphDst[bs] + 1);
                nid++;
                bs++;
            }
        }
        while (as < ae) {
            neighbors[i][nid] = -(graphDst[as] + 1);
            as++;
            nid++;
        }
        while (bs < be) {
            neighbors[i][nid] = (graphDst[bs] + 1);
            bs++;
            nid++;
        }
    }
}

int OIPSimRank::getIntersection(int a, int b) {
    int as = graphSrc[a], ae = graphSrc[a + 1];
    int bs = graphSrc[b], be = graphSrc[b + 1];
    int count = 0;
    while (as < ae && bs < be) {
        if (graphDst[as] < graphDst[bs])
            as++;
        else if (graphDst[as] == graphDst[bs]) {
            count++;
            as++;
            bs++;
        } else
            bs++;
    }
    return count;
}

void OIPSimRank::OP(int u, int iter, double *pSum) {
    double *outP = new double[maxVertexId];//outP[i] denotes outP[u][i]
    bool *isComp = new bool[maxVertexId];
    memset(isComp, 0, sizeof(bool) * maxVertexId);
    memset(outP, 0, sizeof(double) * maxVertexId);
    for (int i = 0; i < maxVertexId; i++) {// for each edge in mst
        int w = mstSrc[i];
        int z = mstDst[i];
        if (graphSrc[z + 1] == graphSrc[z])
            continue;
        if (u > z && isHalf)
            continue;

        if (mstSrc[i] == maxVertexId || !isComp[w]) {// edge directly from node '#'
            for (int k = graphSrc[z]; k < graphSrc[z + 1]; k++)
                outP[z] += pSum[graphDst[k]];
        } else {//given w, compute z
            outP[z] = outP[w];
            int nid = 0;
            while (neighbors[i][nid] != 0) {
                int temp = neighbors[i][nid];
                nid++;
                if (temp > 0) {
                    outP[z] += pSum[temp - 1];
                } else {
                    outP[z] -= pSum[-temp - 1];
                }
            }
        }
        isComp[z] = true;
        if (u == z)
            srvalue[1 - (iter & 1)][u][z] = 1;
        else if (graphSrc[u] == graphSrc[u + 1] || graphSrc[z] == graphSrc[z + 1])
            srvalue[1 - (iter & 1)][u][z] = 0;
        else
            srvalue[1 - (1 & iter)][u][z] =
                    outP[z] * decayFactor / (graphSrc[u + 1] - graphSrc[u]) / (graphSrc[z + 1] - graphSrc[z]);
    }
    delete[]outP;
}
