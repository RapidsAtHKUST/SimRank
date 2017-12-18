#ifndef __GSMANAGER_H__
#define __GSMANAGER_H__

#include "../config.h"
#include "gsinterface.h"

/**
 * TODO: To be I/O efficient gs manager.
 */
class GSManager : public GSInterface {
public:
    GSManager(int sn, int mvid) : sampleGraphNum(sn), maxVertexId(mvid) {
        sampledGraph = new int *[sn];
        for (int i = 0; i < sn; ++i) {
            sampledGraph[i] = new int[maxVertexId];
            memset(sampledGraph[i], -1, sizeof(int) * maxVertexId);
        }
    }

    ~GSManager() override {
        for (int i = 0; i < sampleGraphNum; ++i)
            delete[] sampledGraph[i];
        delete[] sampledGraph;
    }

    void insertEdge(int sid, int src, int dst) override {
        sampledGraph[sid][src] = dst;
    }

    /**
     * Analyze features of the sampled graphs.
     * 1. the number of no incoming vertexes.
     * 2. the number of components.
     */
    void analysis() override {
    }

    /**
     * computeSimRank by traversing the index.
     */
    void computeSimrank(int sid, vector<SimRankValue> &sim, map<int, vector<int> *> &timestamp, int maxSteps, double df,
                        int qv) {
        char *isVisited = new char[maxVertexId];
        memset(isVisited, 0x7f, sizeof(char) * maxVertexId);
        int count = 0, comp = 0;
        int *compByStep = new int[maxSteps + 1];
        memset(compByStep, 0, sizeof(int) * maxSteps + sizeof(int));

        for (int vid = 0; vid < maxVertexId; ++vid) {
            if (isVisited[vid] == 0x7f) {
                /* bfs here. */
                int curv = vid, next;
                int path[maxSteps + 1];
                int step = 0;
                int lastStep = 0;
                int tail = 0;
                bool isFull = false;
                path[tail++] = vid;
                isVisited[vid] = 0;
                while (step <= maxSteps + lastStep) {
                    step++;
                    next = sampledGraph[sid][curv];
//					printf("begin visit: %d, steps=%d\n", count, step);
                    if (next == -1) break;

                    path[tail++] = next;
                    if (tail == maxSteps + 1) {
                        tail = 0;
                        isFull = true;
                    }
                    if (isVisited[next] == 0x7f) {
                        isVisited[next] = step; //record the start time
                        lastStep = step;
                    }

                    if (timestamp.find(next) != timestamp.end()) {
                        int len = timestamp[next]->size();
                        for (int j = 0; j < len; j++) {
                            int ts = (*timestamp[next])[j];
                            int idx = tail - 1 - ts;
                            if (isFull && idx < 0) {
                                idx += maxSteps + 1;
                            }
                            if (idx >= 0 && isVisited[path[idx]] == step - ts && path[idx] != qv) {
                                comp++;
                                compByStep[ts]++;
                                sim[path[idx]].setVid(path[idx]);
                                /* non-first meeting guarantee. */
//                                isVisited[path[idx]] *= -1;
                                sim[path[idx]].incValue(pow(df, ts));
                            }
                        }
                    }
                    curv = next;
                }
                count += step;
            }
        }
        int estStep = 0;
        printf("sim_comp=%d real_steps=%d. details: ", comp, count);
        for (int j = 0; j <= maxSteps; ++j) {
            printf(" %d", compByStep[j]);
            estStep += j * compByStep[j];
        }
        printf(" total=%d\n", estStep);
        delete[] isVisited;
    }

private:
    int sampleGraphNum;
    int maxVertexId;
    int **sampledGraph;
};

#endif
