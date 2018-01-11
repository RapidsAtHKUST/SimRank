//
// Created by yche on 12/18/17.
//

#ifndef TSF_YCHETSF_H
#define TSF_YCHETSF_H

#include <vector>
#include "util/sfmt_based_rand.h"

using namespace std;

class YcheTSF {

private:
    int sampleNum;
    int sampleQueryNum;
    int maxSteps;
    double c;
    SFMTRand rand;

    // reversed graph csr representation
    int n;
    vector<int> off;
    vector<int> dst_v;

    vector<vector<int>> owg_arr;
public:
    double querySinglePair(int u, int v);

    YcheTSF(int sampleNum, int sampleQueryNum, int maxSteps, double c, const vector<int> &graphSrc,
            const vector<int> &graphDst);

private:
    void BuildIndex();
};

#endif //TSF_YCHETSF_H
