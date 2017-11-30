#include "PartialSR.h"

void PartialSR::run(int qv, int k) {
    if (isInit == false) {
        isInit = true;
        initialize();
    } else {
        char filepath[125];
        sprintf(filepath, "dataset/%s/index/partialsr/partialsr.idx", graphName);
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

void PartialSR::initialize() {
    int iter = 0;
    int max_n = maxVertexId - 1;
    bool *is_comp = new bool[maxVertexId];
    double *partial_sums = new double[maxVertexId];
    Time timer;
    timer.start();
    while (iter < maxSteps) {
        for (int a = 0; a <= max_n; ++a) {
            memset(is_comp, 0, sizeof(bool) * (maxVertexId));
            for (int b = 0; b <= max_n; ++b) {
                if (a == b) continue;
                if (isHalf && a > b) continue;
                int size_a = graphSrc[a + 1] - graphSrc[a];
                int size_b = graphSrc[b + 1] - graphSrc[b];
                for (int i = graphSrc[b]; i < graphSrc[b + 1]; ++i) {
                    if (!is_comp[graphDst[i]]) {
                        is_comp[graphDst[i]] = true;
                        double tmp = 0;
                        for (int j = graphSrc[a]; j < graphSrc[a + 1]; ++j) {
                            int sidx = graphDst[j] < graphDst[i] ? graphDst[j] : graphDst[i];
                            int didx = graphDst[j] < graphDst[i] ? graphDst[i] : graphDst[j];
                            tmp += srvalue[iter & 1][sidx][didx];
                        }
                        partial_sums[graphDst[i]] = tmp;
                    }
                    srvalue[1 - (iter & 1)][a][b] += partial_sums[graphDst[i]];
                }
                double tmp = (size_a * size_b / decayFactor);
                if (tmp > 0.0)
                    srvalue[1 - (iter & 1)][a][b] /= tmp;
                else
                    srvalue[1 - (iter & 1)][a][b] = 0;
            }
        }
        ++iter;
        for (int i = 0; i <= max_n; ++i) {//don't forget this
            for (int j = 0; j <= max_n; ++j) {
                if (i != j)
                    srvalue[1 - (iter & 1)][i][j] = 0;
            }
        }

    }
    timer.stop();
    printf("time cost for while loop of accur: %.5lf\n", timer.getElapsedTime());
    char filepath[125];
    sprintf(filepath, "dataset/%s/index/partialsr/partialsr.idx", graphName);
    FILE *fp = fopen(filepath, "wb");
    for (int i = 0; i < maxVertexId; ++i) {
        fwrite(srvalue[maxSteps & 1][i], sizeof(double), maxVertexId, fp);
    }
    fclose(fp);
    delete[] partial_sums;
}
