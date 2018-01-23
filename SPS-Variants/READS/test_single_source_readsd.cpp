//
// Created by yche on 12/17/17.
//

#include <cmath>

#include <iostream>
#include <chrono>

#include "algorithm/reads.h"
#include "util/sample_num.h"
#include "algorithm/readsd.h"

#include "ground_truth/graph_yche.h"
#include "ground_truth/simrank.h"

using namespace std;
using namespace std::chrono;

// usage and example:
// g++ reads*.h reads*.cpp timer.h -O3 -w -std=c++11 test.cpp -I . && ./a.out hp.data 3133
int main(int argc, char **argv) {
    // 1st: eps, delta and c for computing sample number
    double eps = 0.01;
    double delta = 0.01;
    double c = 0.6;
    int r = compute_reads_sample_num(eps, delta, c);
    cout << "sample num:" << r << endl;

    // 2nd: t(max length), n
    int t = 10;
    auto data_name = string(argv[1]);
    auto full_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + data_name + ".txt";
    GraphYche g_gt(full_path);
    int n = g_gt.n;
    cout << "vertex num:" << n << endl;
    auto *ansVal = new double[n];

    // 3rd: construct index
    readsd i2(const_cast<char *>(full_path.c_str()), n, r, c, t);

    // 4th: query (vertex,v)
    int vertex = atoi(argv[2]);
    int v = atoi(argv[3]);

    auto tmp_start = high_resolution_clock::now();
    i2.queryAll(vertex, ansVal);
    auto tmp_end = high_resolution_clock::now();
    cout << "query time:" << duration_cast<microseconds>(tmp_end - tmp_start).count() / pow(10, 6) << " s\n";
    cout << "result:" << ansVal[v] << "\n";

    // ground truth
    if (n < 10000) {
        TruthSim ts(data_name, g_gt, c, 0.01);
        auto max_err = 0.0;
        cout << "err:" << abs(ts.sim(vertex, v) - ansVal[v]) << endl;
    }
    delete[] ansVal;
}