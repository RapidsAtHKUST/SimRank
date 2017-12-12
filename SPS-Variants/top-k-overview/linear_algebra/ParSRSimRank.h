#ifndef _PARSRSIMRANK_H_
#define _PARSRSIMRANK_H_

#include "../config.h"
#include "../simrankmethod.h"

/*
 * implementing SimRank PSR
 * Title:  Yu PVLDB2015
 */
class ParSRSimRank : public SimRankMethod {
public:
    ParSRSimRank(int maxsteps, double decayfactor, int *graph_src, int *graph_dst,
                 int *orig_graph_src, int *orig_graph_dst, int mvid, int type_) :
            graphSrc(graph_src), graphDst(graph_dst), orig_graphSrc(orig_graph_src),
            orig_graphDst(orig_graph_dst), maxVertexId(mvid), type(type_),
            SimRankMethod(maxsteps, 0, decayfactor) {
    };

    ~ParSRSimRank() {

    };

    virtual void run(int qv, int k);

    void Par_SR(int qv, int topk);

    void PrunPar_SR(int qv, int topk);

private:
    void save(vector<SimRankValue> &res, int k) {
        sort(res.begin(), res.end());
        results.clear();
        for (int i = 0; i < k; ++i) {
            results.push_back(res[i]);
        }
    }

private:
//	to store reversed graph
    int *graphSrc;
    int *graphDst;
    int maxVertexId;
    int *orig_graphSrc;
    int *orig_graphDst;
    int type;

};

#endif 

