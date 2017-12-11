#ifndef __TOPSIM_H__
#define __TOPSIM_H__

#include "config.h"
#include "simrankmethod.h"
#include "myqueue.hpp"
#include "simmap.hpp"

class TopSimFamily : public SimRankMethod {
public:
    TopSimFamily(int maxsteps, int samplenum, double decayfactor, int *graph_src, int *graph_dst,
                 int *orig_graph_src, int *orig_graph_dst, int mvid, int type_) :
            SimRankMethod(maxsteps, samplenum, decayfactor), type(type_),
            graphSrc(graph_src), graphDst(graph_dst), orig_graphSrc(orig_graph_src), orig_graphDst(orig_graph_dst),
            maxVertexId(mvid) {}

    ~TopSimFamily() {}

    virtual void run(int qv, int k);

    /* preprocess -- build index */
    void initialize();

private:
    /* query process -- */
    void topsim_sm(int qv, int k);

    void trun_topsim_sm(int qv, int k);

    void prio_topsim_sm(int qv, int k);

    void getSourceNode(int qv, int depth, MyQueue *queue, int *levelNodeCount);

    void getPrioritySourceNode(int qv, int depth, MyQueue *queue, int *levelNodeCount, int priority);

    SimMap topsim_sm_computation(int qv, int pathLen, int *sourceNodes, int size, int *levelNodeCount);

    SimMap
    trun_topsim_sm_computation(int qv, int pathLen, int *sourceNodes, int size, int *levelNodeCount, int highDegree,
                               double eta);

    SimMap
    prio_topsim_sm_computation(int qv, int pathLen, int *sourceNodes, int size, int *levelNodeCount, int priority);

    int binarySearch(vector<SimMap> currentMap, int vid);

    void clearMap(vector<SimMap> &currentMap, int idx) {
        int size = currentMap.size();
        //printf("currentMap size =%d, idx=%d\n", size,idx);
        for (int i = 0; i < size; ++i) {
            if (i != idx) {
                currentMap[i].clear();
            }
        }
    }

    void save(vector<SimRankValue> &res, int k) {
        sort(res.begin(), res.end());
        results.clear();
        for (int i = 0; i < k; ++i) {
            results.push_back(res[i]);
        }
    }

    void visit(int vid, bool *vis, int &count) {
        if (vis[vid] == false) {
            count++;
            vis[vid] = true;
        }
    }

private:
    int type; //1: topsim-sm, 2: trun-topsim-sm, 3: prio-topsim-sm
    int *graphSrc;
    int *graphDst;

    int *orig_graphSrc;
    int *orig_graphDst;

    int maxVertexId;

};

#endif
