#include "topsim.h"
#include "../mytime.h"
#include "sparsehash/dense_hash_map"
#include "sparsehash/sparse_hash_map"

using google::dense_hash_map;
using std::hash;

void TopSimFamily::run(int qv, int k) {
    Time timer;
    printf("Begin query ....\n");
    timer.start();
    switch (type) {
        case 0:
            topsim_sm(qv, k);
            break;
        case 1:
            trun_topsim_sm(qv, k);
            break;
        case 2:
            prio_topsim_sm(qv, k);
            break;
        default:
            printf("Not supported type %d of topsim. [valid ones: 0, 1, 2.]\n", type);
    }
    timer.stop();
    printf("End query cost = %.5lfs\n", timer.getElapsedTime());
}

void TopSimFamily::topsim_sm(int qv, int k) {
    vector<SimRankValue> sim;
    MyQueue *sourceNodeContainer = new MyQueue(maxVertexId);
    int *levelNodeCount = new int[maxSteps];

    /*1. get source nodes via traversing the reversed graph from qv. */
    getSourceNode(qv, maxSteps, sourceNodeContainer, levelNodeCount);

    /*2. do the computation on the original graph. */
    int *sourceNodes = sourceNodeContainer->getContent();
    int size = sourceNodeContainer->length();

//	printf("size=%d\n",size);
//    printf("sourceNodes:\n"); for(int i = 0; i < size; i++) { printf("%d ", sourceNodes[i]); } printf("\n");
    printf("levelCount:\n");
    for (int i = 0; i < maxSteps; i++) { printf("%d ", levelNodeCount[i]); }
    printf("\n");

    Time timer;
    timer.start();
    int pl = maxSteps;
    while (levelNodeCount[pl - 1] == 0) pl--;
    SimMap res = topsim_sm_computation(qv, pl, sourceNodes, size, levelNodeCount);
    timer.stop();
    printf("Time cost for query: %.5lf\n", timer.getElapsedTime());

    /* 3. save answer */
    map<int, double>::iterator iter;
    int cnt = k;
    for (iter = res.EM->begin(); iter != res.EM->end(); ++iter) {
        if (iter->first == qv) continue;
        sim.push_back(SimRankValue(iter->first, iter->second));
        cnt--;
    }
    while (cnt > 0) {
        sim.push_back(SimRankValue(-1, -1));
        cnt--;
    }
    save(sim, k);
    sim.clear();

    res.clear();

    delete sourceNodeContainer;
    delete[] levelNodeCount;

    //printf("Cost for save result: %.5lf\n", timer.getElapsedTime());
}

SimMap TopSimFamily::topsim_sm_computation(int qv, int pathLen, int *sourceNodes, int size, int *levelNodeCount) {
    int processCnt = 0;
    int curLevel = pathLen - 1;
    vector<SimMap> SMList;
    vector<SimMap> mergeMap;

    int ncount = 0;
    bool *vnode = new bool[maxVertexId];
    int vncount = 0;
    memset(vnode, 0, sizeof(bool) * maxVertexId);

    for (int i = size - 1; i >= 0; --i) {
        processCnt++;
        int cur = sourceNodes[i];
        //create SM[cur]
        SimMap sm(cur);
        visit(cur, vnode, vncount);

        //change curLevel here.
        if (processCnt > levelNodeCount[curLevel]) {
            curLevel--;
            clearMap(SMList, -1);
            SMList = mergeMap;
            mergeMap.clear();
            processCnt = 1;
            printf("curV=%d, curLevel=%d, proceccCnt=%d, curMapSize=%lu\n", cur, curLevel, processCnt, SMList.size());
        }

        if (curLevel == pathLen - 1) { //process (n-1)-th neighbors
            int s = graphSrc[cur], e = graphSrc[cur + 1];
            for (int ll = s; ll < e; ++ll) {
                int v = graphDst[ll]; //v in I(cur), O(v) in orig_graphSrc and orig_graphDst
                visit(v, vnode, vncount);
                int os = orig_graphSrc[v], oe = orig_graphSrc[v + 1];
                for (int i = os; i < oe; ++i) {
                    ncount++;
                    int j = orig_graphDst[i];
                    double value = sm.getValue(j);
                    int d = graphSrc[j + 1] - graphSrc[j];
                    visit(j, vnode, vncount);
//                    printf("surf2 %d: pre score=%.5lf, score=%.5lf\n", j, value, value + decayFactor / (e-s)/d);
                    sm.setValue(j, value + decayFactor / (e - s) / d);
                }
            }
            sm.setValue(cur, 1.0);
            mergeMap.push_back(sm);
        } else {
            sort(SMList.begin(), SMList.end());
            int s = graphSrc[cur], e = graphSrc[cur + 1];
            for (int ll = s; ll < e; ++ll) {
                int v = graphDst[ll]; //v in I(cur), O(v) in orig_graphSrc and orig_graphDst
                int idx = binarySearch(SMList, v);
                visit(v, vnode, vncount);
                if (idx < 0) {
                    printf("Cannot find %d's SimMap in current map!\n", v);
                } else {
                    SimMap smv = SMList[idx];
                    map<int, double>::iterator iter;
                    for (iter = smv.EM->begin(); iter != smv.EM->end(); ++iter) {
                        int key = iter->first;
                        double preValue = iter->second;
                        int os = orig_graphSrc[key], oe = orig_graphSrc[key + 1];
                        for (int i = os; i < oe; ++i) {
                            ncount++;
                            int j = orig_graphDst[i];
                            visit(j, vnode, vncount);
                            double value = sm.getValue(j);
                            int d = graphSrc[j + 1] - graphSrc[j];
//                            printf("surf2 %d: pre score=%.5lf, delta score=%.5lf score=%.5lf\n", j, preValue, value, value + preValue*decayFactor / (e-s)/d);
                            sm.setValue(j, value + preValue * decayFactor / (e - s) / d);
                        }
                    }
                }
            }
            sm.setValue(cur, 1.0);
            mergeMap.push_back(sm);
        }
    }
    clearMap(SMList, -1);
    SMList = mergeMap;
    sort(SMList.begin(), SMList.end());
    int idx = binarySearch(SMList, qv);
    if (idx < 0) {
        printf("Cannot find %d's SimMap in current map!\n", qv);
    }
    printf("visit nodes = %d uniqe vis nodes =%d \n", ncount, vncount);
    delete[] vnode;
    clearMap(SMList, idx);
    return SMList[idx];
}

void TopSimFamily::getSourceNode(int qv, int depth, MyQueue *queue, int *levelNodeCount) {
    memset(levelNodeCount, 0, sizeof(int) * maxSteps);
    levelNodeCount[0] = 1;
    queue->push(qv);

    int visCurLevel = 0;
    int curLevel = 0;

    int ts = 0;
    int *vis = new int[maxVertexId];
    memset(vis, -1, sizeof(int) * maxVertexId);
    vis[qv] = ts;

    while (!queue->empty() && curLevel + 1 <
                              depth) { //only expanding level 0 to depth - 2, becasue we only need to get neighbors between 1 to depth - 1.
        int cur = queue->front();
        queue->pop();

        /* expanding on the reversed graph. */
        int s = graphSrc[cur], e = graphSrc[cur + 1];
        while (s < e) {
            int next = graphDst[s++];
            if (vis[next] < ts) {
                vis[next] = ts;
                levelNodeCount[curLevel + 1]++;
                queue->push(next);
            }
        }

        visCurLevel++;

        if (visCurLevel >= levelNodeCount[curLevel]) {
            visCurLevel = 0;
            curLevel++;
            ts++;
        }
    }
    delete[] vis;
}

int TopSimFamily::binarySearch(vector<SimMap> SMList, int key) {
    int left = 0, right = SMList.size() - 1;
    int mid;
    while (left <= right) {
        mid = (left + right) >> 1;
        int tmpKey = SMList[mid].master;
        if (tmpKey == key) return mid;
        if (tmpKey < key) { right = mid - 1; }
        else { left = mid + 1; }
    }
    return -1;
}

void TopSimFamily::trun_topsim_sm(int qv, int k) {
    vector<SimRankValue> sim;
    MyQueue *sourceNodeContainer = new MyQueue(maxVertexId);
    int *levelNodeCount = new int[maxSteps];

    int highDegree = 100;
    double eta = 0.001;

    /*1. get source nodes via traversing the reversed graph from qv. */
    getSourceNode(qv, maxSteps, sourceNodeContainer, levelNodeCount);

    /*2. do the computation on the original graph. */
    int *sourceNodes = sourceNodeContainer->getContent();
    int size = sourceNodeContainer->length();

//	printf("size=%d\n",size);
//    printf("sourceNodes:\n"); for(int i = 0; i < size; i++) { printf("%d ", sourceNodes[i]); } printf("\n");
    printf("levelCount:\n");
    for (int i = 0; i < maxSteps; i++) { printf("%d ", levelNodeCount[i]); }
    printf("\n");

    Time timer;
    timer.start();
    int pl = maxSteps;
    while (levelNodeCount[pl - 1] == 0) pl--;
    SimMap res = trun_topsim_sm_computation(qv, pl, sourceNodes, size, levelNodeCount, highDegree, eta);
    timer.stop();
    printf("Time cost for query: %.5lf\n", timer.getElapsedTime());

    /* 3. save answer */
    map<int, double>::iterator iter;
    int cnt = k;
    for (iter = res.EM->begin(); iter != res.EM->end(); ++iter) {
        if (iter->first == qv) continue;
        sim.push_back(SimRankValue(iter->first, iter->second));
        cnt--;
    }
    while (cnt > 0) {
        sim.push_back(SimRankValue(-1, -1));
        cnt--;
    }
    save(sim, k);
    sim.clear();

    res.clear();

    delete sourceNodeContainer;
    delete[] levelNodeCount;
}

SimMap TopSimFamily::trun_topsim_sm_computation(int qv, int pathLen, int *sourceNodes, int size, int *levelNodeCount,
                                                int highDegree, double eta) {
    int processCnt = 0;
    int curLevel = pathLen - 1;
    vector<SimMap> SMList;
    vector<SimMap> mergeMap;
    int ncount = 0;
    bool *vnode = new bool[maxVertexId];
    memset(vnode, 0, sizeof(bool) * maxVertexId);
    int vncount = 0;
    for (int i = size - 1; i >= 0; --i) {
        processCnt++;
        int cur = sourceNodes[i];
        //create SM[cur]
        SimMap sm(cur);

        visit(cur, vnode, vncount);

        //change curLevel here.
        if (processCnt > levelNodeCount[curLevel]) {
            curLevel--;
            clearMap(SMList, -1);
            SMList = mergeMap;
            mergeMap.clear();
            processCnt = 1;
            printf("curV=%d, curLevel=%d, proceccCnt=%d, curMapSize=%lu\n", cur, curLevel, processCnt, SMList.size());
        }

        if (curLevel == pathLen - 1) { //process (n-1)-th neighbors
            //I(cur) in graphSrc and graphDst
            int s = graphSrc[cur], e = graphSrc[cur + 1];
            for (int ll = s; ll < e; ++ll) {
                int v = graphDst[ll]; //v in I(cur), O(v) in orig_graphSrc and orig_graphDst
                visit(v, vnode, vncount);
                int os = orig_graphSrc[v], oe = orig_graphSrc[v + 1];
                if ((oe - os) * (e - s) > highDegree / (eta * decayFactor))
                    continue;
                for (int i = os; i < oe; ++i) {
                    ncount++;
                    int j = orig_graphDst[i];
                    visit(j, vnode, vncount);
                    double value = sm.getValue(j);
                    int d = graphSrc[j + 1] - graphSrc[j];
//                    printf("surf2 %d: pre score=%.5lf, score=%.5lf\n", j, value, value + decayFactor / (e-s)/d);
                    sm.setValue(j, value + decayFactor / (e - s) / d);
                }
            }
            sm.setValue(cur, 1.0);
            mergeMap.push_back(sm);
        } else {
            sort(SMList.begin(), SMList.end());
//            for(int i = 0; i < SMList.size(); ++i) printf("%d ", SMList[i].master); printf("\n");
            int s = graphSrc[cur], e = graphSrc[cur + 1];
            for (int ll = s; ll < e; ++ll) {
                int v = graphDst[ll]; //v in I(cur), O(v) in orig_graphSrc and orig_graphDst
                visit(v, vnode, vncount);
                int idx = binarySearch(SMList, v);
                if (idx < 0) {
                    printf("Cannot find %d's SimMap in current map!\n", v);
                } else {
                    SimMap smv = SMList[idx];
                    map<int, double>::iterator iter;
                    for (iter = smv.EM->begin(); iter != smv.EM->end(); ++iter) {
                        int key = iter->first;
                        double preValue = iter->second;
                        int os = orig_graphSrc[key], oe = orig_graphSrc[key + 1];

                        // truncate
                        if ((oe - os) * (e - s) > highDegree && preValue < eta)
                            continue;

                        for (int i = os; i < oe; ++i) {
                            ncount++;
                            int j = orig_graphDst[i];
                            visit(j, vnode, vncount);
                            double value = sm.getValue(j);
                            int d = graphSrc[j + 1] - graphSrc[j];
//                            printf("surf2 %d: pre score=%.5lf, delta score=%.5lf score=%.5lf\n", j, preValue, value, value + preValue*decayFactor / (e-s)/d);
                            sm.setValue(j, value + preValue * decayFactor / (e - s) / d);
                        }
                    }
                }
            }
            sm.setValue(cur, 1.0);
            mergeMap.push_back(sm);
        }
    }
    clearMap(SMList, -1);
    SMList = mergeMap;
    sort(SMList.begin(), SMList.end());
    int idx = binarySearch(SMList, qv);
    if (idx < 0) {
        printf("Cannot find %d's SimMap in current map!\n", qv);
    }
    printf("visit nodes = %d, uniqe vis nodes=%d\n", ncount, vncount);
    delete[] vnode;
    clearMap(SMList, idx);
    return SMList[idx];
}

void TopSimFamily::getPrioritySourceNode(int qv, int depth, MyQueue *queue, int *levelNodeCount, int priority) {
    memset(levelNodeCount, 0, sizeof(int) * maxSteps);
    levelNodeCount[0] = 1;
    queue->push(qv);

    int visCurLevel = 0;
    int curLevel = 0;

    int ts = 0;
    int *vis = new int[maxVertexId];
    memset(vis, -1, sizeof(int) * maxVertexId);
    vis[qv] = ts;

    while (!queue->empty() && curLevel + 1 <
                              depth) { //only expanding level 0 to depth - 2, becasue we only need to get neighbors between 1 to depth - 1.
        int cur = queue->front();
        queue->pop();

        /* expanding on the reversed graph. */
        int s = graphSrc[cur], e = graphSrc[cur + 1]; // only expanding H needs here.
        vector<SimRankValue> neighbors;
        for (int i = s; i < e; ++i) {
            int v = graphDst[i];
            int inDeg = graphSrc[v + 1] - graphSrc[v];
            neighbors.push_back(SimRankValue(v, 1.0 / (inDeg + 1)));
        }
        sort(neighbors.begin(), neighbors.end());
//        for(int i = 0; i < neighbors.size(); ++i) printf("%.5lf ", neighbors[i].getValue()); printf("\n");
        int size = (neighbors.size() < priority ? neighbors.size() : priority);
        for (int i = 0; i < size; ++i) {
            int next = neighbors[i].getVid();
            if (vis[next] < ts) {
                vis[next] = ts;
                levelNodeCount[curLevel + 1]++;
                queue->push(next);
            }
        }

        visCurLevel++;

        if (visCurLevel >= levelNodeCount[curLevel]) {
            visCurLevel = 0;
            curLevel++;
            ts++;
        }
    }
    delete[] vis;
}

void TopSimFamily::prio_topsim_sm(int qv, int k) {
    vector<SimRankValue> sim;
    MyQueue *sourceNodeContainer = new MyQueue(maxVertexId);
    int *levelNodeCount = new int[maxSteps];

    int priority = 100;

    /*1. get source nodes via traversing the reversed graph from qv. */
    getPrioritySourceNode(qv, maxSteps, sourceNodeContainer, levelNodeCount, priority);

    /*2. do the computation on the original graph. */
    int *sourceNodes = sourceNodeContainer->getContent();
    int size = sourceNodeContainer->length();

//	printf("size=%d\n",size);
//  printf("sourceNodes:\n"); for(int i = 0; i < size; i++) { printf("%d ", sourceNodes[i]); } printf("\n");
    printf("levelCount:\n");
    for (int i = 0; i < maxSteps; i++) { printf("%d ", levelNodeCount[i]); }
    printf("\n");

    Time timer;
    timer.start();
    int pl = maxSteps;
    while (levelNodeCount[pl - 1] == 0) pl--;
    SimMap res = prio_topsim_sm_computation(qv, pl, sourceNodes, size, levelNodeCount, priority);
    timer.stop();
    printf("Time cost for query: %.5lf\n", timer.getElapsedTime());

    /* 3. save answer */
    map<int, double>::iterator iter;
    int cnt = k;
    for (iter = res.EM->begin(); iter != res.EM->end(); ++iter) {
        if (iter->first == qv) continue;
        sim.push_back(SimRankValue(iter->first, iter->second));
        cnt--;
    }
    while (cnt > 0) {
        sim.push_back(SimRankValue(-1, -1));
        cnt--;
    }
    save(sim, k);
    sim.clear();

    res.clear(); //delete the EM in SimMap

    delete sourceNodeContainer;
    delete[] levelNodeCount;
}

SimMap TopSimFamily::prio_topsim_sm_computation(int qv, int pathLen, int *sourceNodes, int size, int *levelNodeCount,
                                                int priority) {
    long processCnt = 0;
    int curLevel = pathLen - 1;
    int maxMapSize = -1;
    int levelVis = 0;
    vector<SimMap> SMList;
    vector<SimMap> mergeMap;
    int ncount = 0;
    bool *vnode = new bool[maxVertexId];
    memset(vnode, 0, sizeof(bool) * maxVertexId);
    int vncount = 0;
    Time timer;
    timer.start();
    for (int i = size - 1; i >= 0; --i) {
        processCnt++;
        int cur = sourceNodes[i];
        //create SM[cur]
        SimMap sm(cur);
        visit(cur, vnode, vncount);

        //change curLevel here.
        if (processCnt > levelNodeCount[curLevel]) {
            timer.stop();
            printf("curLevel=%d, processCnt=%ld, curSMSize=%lu, maxMapSize=%d, levelVis=%d, cost=%.5lf\n", curLevel,
                   processCnt, mergeMap.size(), maxMapSize, levelVis, timer.getElapsedTime());
            curLevel--;
            clearMap(SMList, -1);
            SMList = mergeMap;
            mergeMap.clear();
            processCnt = 1;
            maxMapSize = -1;
            levelVis = 0;
        }

        if (curLevel == pathLen - 1) { //process (n-1)-th neighbors
            //I(cur) in graphSrc and graphDst
            int s = graphSrc[cur], e = graphSrc[cur + 1];
            for (int ll = s; ll < e; ++ll) {
                int v = graphDst[ll]; //v in I(cur), O(v) in orig_graphSrc and orig_graphDst
                visit(v, vnode, vncount);
                int os = orig_graphSrc[v], oe = orig_graphSrc[v + 1];
//                printf("curV=%d, v=%d deg=%d\n", cur, v, oe-os);
                for (int i = os; i < oe; ++i) {
                    ncount++;
                    levelVis++;
                    int j = orig_graphDst[i];
                    visit(j, vnode, vncount);
                    double value = sm.getValue(j);
                    int d = graphSrc[j + 1] - graphSrc[j];
//                    printf("surf2 %d: pre score=%.5lf, score=%.5lf\n", j, value, value + decayFactor / (e-s)/d);
                    sm.setValue(j, value + decayFactor / (e - s) / d);
                }
            }
            if (sm.size() > maxMapSize) {
                maxMapSize = sm.size();
            }
            sm = sm.prioritize(priority);
            sm.setValue(cur, 1.0);
            mergeMap.push_back(sm);
        } else {
            sort(SMList.begin(), SMList.end());
//            for(int i = 0; i < SMList.size(); ++i) printf("%d ", SMList[i].master); printf("\n");
            int s = graphSrc[cur], e = graphSrc[cur + 1];
            for (int ll = s; ll < e; ++ll) {
                int v = graphDst[ll]; //v in I(cur), O(v) in orig_graphSrc and orig_graphDst
                visit(v, vnode, vncount);
                int idx = binarySearch(SMList, v);
                if (idx < 0) {
//                	printf("Cannot find %d's SimMap in current map!\n", v);
                } else {
                    SimMap smv = SMList[idx];
                    map<int, double>::iterator iter;
                    for (iter = smv.EM->begin(); iter != smv.EM->end(); ++iter) {
                        int key = iter->first;
                        double preValue = iter->second;
                        int os = orig_graphSrc[key], oe = orig_graphSrc[key + 1];

                        for (int i = os; i < oe; ++i) {
                            ncount++;
                            levelVis++;
                            int j = orig_graphDst[i];
                            visit(j, vnode, vncount);
                            double value = sm.getValue(j);
                            int d = graphSrc[j + 1] - graphSrc[j];
//                            printf("surf2 %d: pre score=%.5lf, delta score=%.5lf score=%.5lf\n", j, preValue, value, value + preValue*decayFactor / (e-s)/d);
                            sm.setValue(j, value + preValue * decayFactor / (e - s) / d);
                        }
                    }
                }
            }
            if (sm.size() > maxMapSize) {
                maxMapSize = sm.size();
            }
            if (curLevel > 0)
                sm = sm.prioritize(priority);
            sm.setValue(cur, 1.0);
            mergeMap.push_back(sm);
        }
    }
    timer.stop();
    printf("curLevel=%d, processCnt=%ld, curSMSize=%lu, maxMapSize=%d, levelVis=%d, cost=%.5lf\n", curLevel, processCnt,
           mergeMap.size(), maxMapSize, levelVis, timer.getElapsedTime());
    clearMap(SMList, -1);
    SMList = mergeMap;
    sort(SMList.begin(), SMList.end());
    int idx = binarySearch(SMList, qv);
    if (idx < 0) {
        printf("Cannot find %d's SimMap in current map!\n", qv);
    }
    printf("visit nodes = %d uniqe vis nodes =%d \n", ncount, vncount);
    delete[] vnode;
    clearMap(SMList, idx);
    return SMList[idx];

}

void TopSimFamily::initialize() {
}
