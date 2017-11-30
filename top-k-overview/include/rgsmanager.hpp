#ifndef __RGSMANAGER_H__
#define __RGSMANAGER_H__

#include "config.h"
#include "gsinterface.h"
#include "rsamplegraph.hpp"

using namespace std;

class RGSManager : public GSInterface {
public:
    RGSManager(int sn, int mvid, bool isfm) :
            sampleGraphNum(sn), maxVertexId(mvid), isFm(isfm) {
        rsg = new RSampleGraph *[sn];
        for (int i = 0; i < sn; ++i) {
            rsg[i] = new RSampleGraph(maxVertexId);
        }
    }

    ~RGSManager() {
        for (int i = 0; i < sampleGraphNum; ++i)
            delete rsg[i];
        delete[] rsg;
    }

    void insertEdge(int sid, int src, int dst) {
        /*revserse the edge (src, dst) here. */
        rsg[sid]->addEdge(dst, src);
    }

    void analysis() {
        double totalMemCost = 0;
        for (int i = 0; i < sampleGraphNum; ++i) {
            rsg[i]->preprocess();
            totalMemCost += rsg[i]->getMemSize();
        }
        printf("Index MEM cost: %.5lfMB\n", totalMemCost);
    }

    void
    computeSimrank(int sid, vector<SimRankValue> &sim, map<int, vector<pair<int, int> > *> &timestamp, int maxSteps,
                   double df, int qv, int sqn) {
        map<int, vector<pair<int, int> > *> meetmap;
        map<int, vector<pair<int, int> > *>::iterator iter2;

        double buildCost = 0.0;
        Time timer;
        /* 1. reverse graph */
        map<int, vector<pair<int, int> > *>::iterator iter;
        for (iter = timestamp.begin(); iter != timestamp.end(); ++iter) {
            sort((*(iter->second)).begin(), (*(iter->second)).end());
        }

        timer.start();
        /* 2. compute simrank. */
        int comp = 0;
        int traverse_cnt = 0;
        /* enumerate meeting points here. */

        int *hasMeet = new int[sqn];//hasMeet[i] records whether random walk i has met at vid
        for (iter = timestamp.begin(); iter != timestamp.end(); ++iter) {
            vector<pair<int, int> > *tsv = iter->second;
            int vid = iter->first; /* one meeting points. */
            int idx = 0;
            int stepLim;
            queue<int> cand[2];
            int step = 0, cur, cnt;
            int tsvLen = tsv->size();
            cand[step].push(vid);
            while (idx < tsvLen) {
                stepLim = (*tsv)[idx].first;
                memset(hasMeet, 0, sizeof(int) * sqn);
                cnt = 0;

                while (idx < tsvLen && stepLim == (*tsv)[idx].first) {
                    int randomWalkId = (*tsv)[idx].second;
                    hasMeet[randomWalkId] = 1;
                    cnt++;
                    idx++;
                }
                /* traverse the tree. */
                do {
                    while (cand[step & 1].empty() == false) {
                        cur = cand[step & 1].front();
                        cand[step & 1].pop();
                        traverse_cnt++;

                        if (qv != cur && step == stepLim) {
                            comp += cnt;
                            /* update SimRank */
                            //sim[cur].setVid(cur);
                            /* non-first meeting guarantee. */
                            //sim[cur].incValue(pow(df, step)*cnt);
                            if (meetmap.find(cur) == meetmap.end()) {
                                meetmap[cur] = new vector<pair<int, int> >();
                            }
                            for (int rid = 0; rid < sqn; rid++) {
                                if (hasMeet[rid] == 1) {
                                    meetmap[cur]->push_back(make_pair(stepLim, rid));
                                }
                            }
                        }

                        /* enumerate edge here! */
                        rsg[sid]->expand(cur, cand[(step + 1) & 1]);
                    }
                    step++;
                } while (step <= stepLim);
            }
        }
        //compute simrank value with first-meeting or not
        for (iter2 = meetmap.begin(); iter2 != meetmap.end(); iter2++) {
            int cur = iter2->first;
            memset(hasMeet, 0, sizeof(int) * sqn);
            sort((*(iter2->second)).begin(), (*(iter2->second)).end());
            vector<pair<int, int> > *mm = iter2->second;
            int mmLen = mm->size();
            int idx = 0;
            while (idx < mmLen) {
                int stepNum = (*mm)[idx].first;
                int rid = (*mm)[idx].second;
                if (isFm) {
                    if (hasMeet[rid] == 0) {
                        sim[cur].setVid(cur);
                        sim[cur].incValue(pow(df, stepNum));
                        hasMeet[rid] = 1;
                    } else {
                    }
                } else {
                    sim[cur].setVid(cur);
                    sim[cur].incValue(pow(df, stepNum));
                }
                idx++;
            }

        }
        timer.stop();
        buildCost = timer.getElapsedTime();
        delete[] hasMeet;
        for (iter2 = meetmap.begin(); iter2 != meetmap.end(); iter2++) {
            delete iter2->second;
        }
    }

private:
    int sampleGraphNum;
    int maxVertexId;
    bool isFm;
    RSampleGraph **rsg;
};

#endif
