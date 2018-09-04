#ifndef _EVALUATE_H_
#define _EVALUATE_H_

#include <cmath>

#include "util.h"

/*
topk contains top-k (or |V|-1) estimated (id, value) pairs;
realMap contains the ground truth (id, value) pairs computed by the Power Method.
*/
double calAbsError(unordered_map<int, double> &topk, unordered_map<int, double> &realMap) {
    double max_err = 0;
    unordered_map<int, double>::iterator realItor;
    for (realItor = realMap.begin(); realItor != realMap.end(); realItor++) {
        int v = realItor->first;
        if (topk.find(v) != topk.end()) {
            if (abs(realItor->second - topk.find(v)->second) > max_err)
                max_err = abs(realItor->second - topk.find(v)->second);
        } else {
            if (realItor->second > max_err)
                max_err = realItor->second;
        }
    }
    return max_err;
}

/*
Compute Precision@k
topK contains the ids of the top-k highest SimRank w.r.t. the query node;
realList contains the true top-k ids, computed by the Power Method or polled by the Monte Carlo Method.
*/
double calPrecision(vector<int> &topK, vector<int> &realList, int k) {
    int hitCount = 0;
    for (int i = 0; i < realList.size(); i++) {
        for (int j = 0; j < topK.size(); j++) {
            if (topK[j] == realList[i]) {
                hitCount++;
                break;
            }
        }
    }
    return hitCount / (double) k;
}

/*
Compute NDCG@k
topK contains the ids of the top-k highest SimRank w.r.t. the query node;
realTopK contains the true top-k ids, computed by the Power Method or polled by the Monte Carlo Method;
real_map contains all the true (id, value) pairs.
*/
double calNDCG(vector<int> &topK, vector<int> &realTopK, unordered_map<int, double> &real_map, int k) {
    double Zp = 0;
    int sz_real = realTopK.size();
    for (int i = 1; i <= min(k, sz_real); i++) {
        double tempSim = real_map[realTopK[i - 1]];
        Zp += (pow(2, tempSim) - 1) / (log(i + 1) / log(2));
    }

    double NDCG = 0;
    int sz_topk = topK.size();
    for (int i = 1; i <= min(k, sz_topk); i++) {
        double tempSim = real_map[topK[i - 1]];
        NDCG += (pow(2, tempSim) - 1) / (log(i + 1) / log(2));
    }
    if (Zp == 0)
        NDCG = 1;
    else
        NDCG = NDCG / Zp;
    return NDCG;
}

/*
Compute Tau@k
topK contains the ids of the top-k highest SimRank w.r.t. the query node;
real_map contains all the true (id, value) pairs.
*/
double calTau(vector<int> &topK, unordered_map<int, double> &real_map, int k) {
    int discordantNum = 0, cordantNum = 0;
    for (int i = 0; i < k; i++) {
        for (int j = i + 1; j < k; j++) {
            if (real_map[topK[i]] >= real_map[topK[j]])
                cordantNum++;
            else
                discordantNum++;
        }
    }
    return (cordantNum - discordantNum) * 2 / (double) (k * (k - 1));
}

/* 
Compute absolute error for each query node in qfile.
The estimated values are in res_dir.
The ground truth or polled ground truth is in gt_dir. 
*/
void evaluate(string qfile, string res_dir, string gt_dir) {
    vector<int> vq;
    readfile(qfile, vq);

    double avg_abserr = 0;
    for (int i = 0; i < vq.size(); i++) {
        int v = vq[i];
        stringstream ss_res, ss_gt;
        ss_res << res_dir << "/" << v << ".txt";
        ss_gt << gt_dir << "/" << v << ".txt";
        string s_res = ss_res.str();
        string s_gt = ss_gt.str();

        unordered_map<int, double> topk, realmap;
        cout << v << " : " << readfile(s_res, topk) << " , " << readfile(s_gt, realmap) << " ";
        double abserr = calAbsError(topk, realmap);
        cout << abserr << endl;
        avg_abserr += abserr;
    }
    cout << "average absolute error over " << vq.size() << " queries: " << avg_abserr / (double) vq.size() << endl;
}

/*
Compute Precision@k, NDCG@k and Tau@k for each query node in qfile.
The estimated values are in res_dir.
The ground truth or polled ground truth is in gt_dir.
*/
void evaluateTopK(string qfile, string res_dir, string gt_dir, int k) {
    vector<int> vq;
    readfile(qfile, vq);

    double avg_pres = 0, avg_ndcg = 0, avg_tau = 0;
    for (int i = 0; i < vq.size(); i++) {
        int v = vq[i];
        stringstream ss_res, ss_gt;
        ss_res << res_dir << "/" << v << ".txt";
        ss_gt << gt_dir << "/" << v << ".txt";
        string s_res = ss_res.str();
        string s_gt = ss_gt.str();

        vector<int> topk, real;
        unordered_map<int, double> realmap;
        cout << v << " : " << readfile(s_res, topk, k) << " , " << readfile2(s_gt, real, k) << " , "
             << readfile(s_gt, realmap) << endl;
        double pres = calPrecision(topk, real, k);
        cout << "Precision@" << k << " : " << pres << endl;
        avg_pres += pres;
        double ndcg = calNDCG(topk, real, realmap, k);
        cout << "NDCG@" << k << " : " << ndcg << endl;
        avg_ndcg += ndcg;
        double tau = calTau(topk, realmap, k);
        cout << "Tau@" << k << " : " << tau << endl;
        avg_tau += tau;
    }
    cout << "average Precision@" << k << " over " << vq.size() << " queries: " << avg_pres / (double) vq.size() << endl;
    cout << "average NDCG@" << k << " over " << vq.size() << " queries: " << avg_ndcg / (double) vq.size() << endl;
    cout << "average Tau@" << k << " over " << vq.size() << " queries: " << avg_tau / (double) vq.size() << endl;
}

#endif