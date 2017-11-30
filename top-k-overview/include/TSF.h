#ifndef __TSF_H__
#define __TSF_H__

#include "config.h"
#include "simrankmethod.h"
#include "gsmanager.hpp"
#include "rgsmanager.hpp"

class TSF : public SimRankMethod {
public:
    TSF(int maxsteps, int samplenum, double decayfactor, int samplequerynum, int *graph_src, int *graph_dst, int mvid,
        int udisk, char *sgpath, bool isinit = false, bool isFm = false) :
            SimRankMethod(maxsteps, samplenum, decayfactor), sampleQueryNum(samplequerynum),
            isInit(isinit), graphSrc(graph_src), graphDst(graph_dst), uDisk(udisk), maxVertexId(mvid) {
        gsm = new RGSManager(samplenum, mvid, isFm);
    }

    ~TSF() {
        delete gsm;
    }

    virtual void run(int qv, int k);

    void update(int src, int dst) {
        gsm->update(src, dst);
    }

    void setSampleQueryNum(int sqn) { sampleQueryNum = sqn; }

    /* preprocess -- build index */
    void initialize();

private:
    /* query process -- */
    void query(int qv, int k);

    void updateSimrank(vector<SimRankValue> &sim, int **randwalk, int sid, int qv);

    void save(vector<SimRankValue> &res, int k) {
//            printf("here is srgs %d", res.size());
        sort(res.begin(), res.end());
        results.clear();
        for (int i = 0; i < k; ++i) {
            res[i].setValue(res[i].getValue() / (sampleNum * sampleQueryNum));
            results.push_back(res[i]);
        }
    }

private:
    int sampleQueryNum;
    bool isInit;
    int *graphSrc;
    int *graphDst;
    int uDisk;
    int maxVertexId;
    GSInterface *gsm;
};

#endif
