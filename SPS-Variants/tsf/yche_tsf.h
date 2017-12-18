//
// Created by yche on 12/18/17.
//

#ifndef TSF_YCHETSF_H
#define TSF_YCHETSF_H

#include <vector>

using namespace std;

class YcheTSF {

private:
    int sampleNum;
    int sampleQueryNum;
    int maxSteps;
    double c;

    // reversed graph csr representation
    int n;
    vector<int> graphSrc;
    vector<int> graphDst;

    vector<vector<int>> owg_arr;
public:
    double querySinglePair(int u, int v);

    YcheTSF(int sampleNum, int sampleQueryNum, int maxSteps, double c, const vector<int> &graphSrc,
            const vector<int> &graphDst);

private:
    void BuildIndex();
};

#endif //TSF_YCHETSF_H
