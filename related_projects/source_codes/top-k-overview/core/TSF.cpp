#include "TSF.h"
#include <mytime.h>

void TSF::run(int qv, int k) {
    Time timer;
    if (isInit == false) {
        printf("Begin Initializing....\n");
        timer.start();
        isInit = true;
        initialize();
        timer.stop();
        printf("End Initializing cost=%.5lfs\n", timer.getElapsedTime());
        timer.reset();
        timer.start();
        gsm->analysis();
        timer.stop();
        printf("End Analyzing cost=%.5lfs\n", timer.getElapsedTime());
    }
    printf("Begin query ....\n");
    timer.start();
    query(qv, k);
    timer.stop();
    printf("End query cost = %.5lfs\n", timer.getElapsedTime());
}

void TSF::query(int qv, int k) {
    Random rand;
    vector<SimRankValue> sim;
    sim.resize(maxVertexId);
    int **randwalk = new int *[sampleQueryNum];
    for (int i = 0; i < sampleQueryNum; ++i) {
        randwalk[i] = new int[maxSteps];
    }
    Time timer;
    double updateCost = 0.0;
    timer.start();
    for (int sid = 0; sid < sampleNum; ++sid) {
        for (int qsid = 0; qsid < sampleQueryNum; ++qsid) {
            /* 1. sample a random walk */
            int curv = qv;
            for (int s = 0; s < maxSteps; ++s) {
                int len = graphSrc[curv + 1] - graphSrc[curv];
                if (len > 0) {
                    randwalk[qsid][s] = graphDst[graphSrc[curv] + (rand.getRandom() % len)];
                    curv = randwalk[qsid][s];
                } else {
                    randwalk[qsid][s] = -1;
                    break;
                }
            }
        }
        if ((sid + 1) % 100 == 0) {
            printf("%d samples are simulated.\n", sid + 1);
        }

        /* 2. core: compute the similarity */
        updateSimrank(sim, randwalk, sid, qv);
    }
    timer.stop();
    updateCost += timer.getElapsedTime();
    printf("UpdateCost=%.5lf\nBegin save answer...\n", updateCost);
    timer.reset();
    timer.start();
    for (int i = 0; i < sampleQueryNum; ++i)
        delete[] randwalk[i];
    delete[] randwalk;

    //printf("size=%u k=%d\n", sim.size(), k);
    Time tt;
    tt.start();
    vector<SimRankValue> sim2;
    int size = sim.size();
    int cnt = 0;
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
    tt.stop();
    printf("Cost for transferring: %.5lf %lu\n", tt.getElapsedTime(), sim2.size());

    save(sim2, k);
    sim.clear();
    sim2.clear();
    timer.stop();
    printf("Cost for save result: %.5lf\n", timer.getElapsedTime());
}

void TSF::updateSimrank(vector<SimRankValue> &sim, int **randwalk, int sid, int qv) {
    /* implement the solution as traversing the graph. */
    /* 1. compute the timestamp first. */
    map<int, vector<pair<int, int> > *> timestamp;// pair<stepid, walkid>
    for (int i = 0; i < sampleQueryNum; ++i) {
        for (int s = 0; s < maxSteps; ++s) {
            int vid = randwalk[i][s];
            if (vid == -1) break;
            if (timestamp.find(vid) == timestamp.end()) {
                timestamp[vid] = new vector<pair<int, int> >();
            }
            timestamp[vid]->push_back(make_pair(s + 1, i));
        }
    }
    /* 2. update the simrank. */
    gsm->computeSimrank(sid, sim, timestamp, maxSteps, decayFactor, qv, sampleQueryNum);
    map<int, vector<pair<int, int> > *>::iterator
            iter;
    for (iter = timestamp.begin(); iter != timestamp.end(); ++iter) {
        delete iter->second;
    }
}

/**
 * Build index here.
 */
void TSF::initialize() {
    Random rand;
    if (uDisk == 0) {
        for (int i = 0; i < maxVertexId; ++i) {
            for (int sid = 0; sid < sampleNum; ++sid) {
                int rnum = rand.getRandom();
                /* insert sampled edge (sid, i, graph[i][rnum % graph[i].size()]) into a centralized index management*/
                if (graphSrc[i] != graphSrc[i + 1]) {
                    int idx = rnum % (graphSrc[i + 1] - graphSrc[i]);
                    //                printf("sampled edge in initialzation: (%d, %d)\n", i, graph[i][idx]);
                    gsm->insertEdge(sid, i, graphDst[graphSrc[i] + idx]);
                }
            }
        }
    } else {
        printf("invalid srgs type\n");
    }
}
