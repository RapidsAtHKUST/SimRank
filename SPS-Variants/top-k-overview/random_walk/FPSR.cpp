#include "FPSR.h"

FPSR::FPSR(int maxSteps, int sampleNum, double decayfactor, int *graph_src, int *graph_dst, int mvid, char *graph,
           bool isinit) :
        SimRankMethod(maxSteps, sampleNum, decayfactor), graphSrc(graph_src), graphDst(graph_dst), maxVertexId(mvid),
        isInit(isinit) {
    isLoad = false;
    strcpy(graphName, graph);
    index = new FingerPrint *[sampleNum];
    for (int i = 0; i < sampleNum; ++i) {
        index[i] = new FingerPrint(mvid);
    }
}


void FPSR::indexBuilder(int sn) {
    printf("Index building ... %d\n", sn);
    vector<int> pathEnd;
    vector<int> nextIn;
    pathEnd.resize(maxVertexId, 0);
    nextIn.resize(maxVertexId, 0);

    for (int i = 0; i < maxVertexId; ++i) {
        pathEnd[i] = i;
    }

    for (int s = 1; s <= maxSteps; ++s) {
        for (int i = 0; i < maxVertexId; ++i) {
            int startIndex = graphSrc[i];
            int endIndex = graphSrc[i + 1];
            int size = endIndex - startIndex;
            if (endIndex == startIndex) {
                nextIn[i] = -1;
                continue;
            }
            nextIn[i] = graphDst[abs(randomGenerator.getRandom()) % (size) + startIndex];
        }

        for (int i = 0; i < maxVertexId; ++i) {
            if (pathEnd[i] != -1) {
                pathEnd[i] = nextIn[pathEnd[i]];
            }
        }

        /*detecting the meeting points*/
        for (int i = 0; i < maxVertexId; ++i) {
            nextIn[i] = -1; //records the smallest vertex id that meeting at vertex i;
        }

        for (int i = 0; i < maxVertexId; ++i) {
            if (pathEnd[i] != -1) {
                if (nextIn[pathEnd[i]] == -1) {
                    nextIn[pathEnd[i]] = i;
                } else if (nextIn[pathEnd[i]] < i) {
                    index[sn]->addEdge(i, nextIn[pathEnd[i]], s);
                    pathEnd[i] = -1;
                } else if (nextIn[pathEnd[i]] != i) {
                    index[sn]->addEdge(nextIn[pathEnd[i]], i, s);
                    pathEnd[nextIn[pathEnd[i]]] = -1;
                    nextIn[pathEnd[i]] = i;
                }
            }
        }
    }
}

void FPSR::initialize() {
    char sgpath[125];
    if (isInit == false) {
        Time timer;
        double saveCost = 0.0;
        for (int i = 0; i < sampleNum; ++i) {
            sprintf(sgpath, "dataset/%s/index/fpsr/fingerprint_%d", graphName, i);
            indexBuilder(i);
            timer.reset();
            timer.start();
            index[i]->save(sgpath);
            timer.stop();
            saveCost += timer.getElapsedTime();
        }
        printf("Save index cost=%.5lf\n.", saveCost);
        isInit = true;
        isLoad = true;
    } else if (isLoad == false) {
        for (int i = 0; i < sampleNum; ++i) {
            sprintf(sgpath, "dataset/%s/index/fpsr/fingerprint_%d", graphName, i);
            index[i]->read(sgpath);

        }
        isLoad = true;
    }
}

void FPSR::run(int qv, int k) {
    /* priority in C++ is the max heap using the strict weaking order. 
     * return the last element when top() is called. */
    //priority_queue<SimRankValue,vector<SimRankValue>,ComparisonClass> pq;
    //vector<double> valueOfSimRank;
    vector<SimRankValue> sim;
    sim.resize(maxVertexId);

    Time timer;
    timer.start();
    initialize();
    timer.stop();
    printf("Building index cost: %.5lf.\n", timer.getElapsedTime());

    timer.reset();
    timer.start();
    calSimRank(qv, sim);
    timer.stop();
    printf("UpdateCost=%.5lf\n", timer.getElapsedTime());

    timer.reset();
    timer.start();
    vector<SimRankValue> sim2;
    int size = sim.size(), cnt = 0;
    for (int i = 0; i < size; ++i) {
        if (sim[i].getValue() > 0.0) {
            sim2.push_back(sim[i]);
            cnt++;
        }
    }
    while (cnt < k) {
        cnt++;
        sim2.push_back(SimRankValue());
    }
    save(sim2, k);
    sim.clear();
    sim2.clear();
    timer.stop();
    printf("Cost of a single query: %.5lf\n", timer.getElapsedTime());
}

void FPSR::calSimRank(int queryv, vector<SimRankValue> &value) {
    map<int, int> queryPath;
    int *cand = new int[maxVertexId];
    int tail = 0;
    for (int idx = 0; idx < sampleNum; ++idx) {
        queryPath.clear();
        index[idx]->getMeetPoint(queryv, queryPath);
        tail = 0;
        index[idx]->getCand(queryv, cand, tail);
        for (int i = 0; i < tail; ++i) {
            if (cand[i] == queryv) { continue; }
            int meettime = index[idx]->calMeetTime(cand[i], queryPath);
            value[cand[i]].setVid(cand[i]);
            value[cand[i]].incValue(pow(decayFactor, meettime));
        }
    }
    delete[] cand;
    for (int i = 0; i < maxVertexId; ++i) {
        value[i].div((double) sampleNum);
    }
}
