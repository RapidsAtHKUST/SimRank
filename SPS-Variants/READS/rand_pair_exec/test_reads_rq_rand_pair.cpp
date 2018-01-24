//
// Created by yche on 12/17/17.
//

#include <cmath>

#include <iostream>
#include <chrono>

#include "../algorithm/readsrq.h"
#include "../util/sample_num.h"
#include "../algorithm/readsd.h"

#include "../ground_truth/graph_yche.h"
#include "../ground_truth/simrank.h"
#include "../ground_truth/random_pair_generator.h"

using namespace std;
using namespace std::chrono;

// usage and example:
// g++ reads*.h reads*.cpp timer.h -O3 -w -std=c++11 test.cpp -I . && ./a.out hp.data 3133
int main(int argc, char **argv) {
    // 1st: eps, delta and c for computing sample number
    double eps = 0.01;
    double delta = 0.01;
    double c = 0.6;
    int r = 100;
    int rq = compute_reads_rq_num(eps, delta, c, r);
    cout << "sample num:" << r << ", on-line rand-walk:" << rq << endl;

    // 2nd: t(max length), n
    int t = 10;
    auto data_name = string(argv[1]);
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
    auto full_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + data_name + ".txt";
    GraphYche g_gt(full_path);
    int n = g_gt.n;
    cout << "vertex num:" << n << endl;

    // 3rd: construct index
    auto start = high_resolution_clock::now();
    readsrq i3(data_name, n, r, rq, c, t);
    auto end = high_resolution_clock::now();
    cout << "construct time:" << duration_cast<microseconds>(end - start).count() / pow(10, 6) << " s\n";

    auto sample_pairs = read_sample_pairs(data_name, pair_num, round_i);
    // 4th: query
    if (n < 10000) {
        double max_err = 0;
        TruthSim ts(data_name, g_gt, c, 0.01);
        auto tmp_start = high_resolution_clock::now();
        auto clock_start = clock();

#pragma omp parallel
        {
            auto *ansVal = new double[n];

#pragma omp for reduction(max:max_err) schedule(dynamic, 100)
            for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
                auto u = sample_pairs[pair_i].first;
                auto v = sample_pairs[pair_i].second;

                auto result = i3.queryOne(u, v, ansVal);
                max_err = max(max_err, abs(ts.sim(u, v) - result));
                if (abs(ts.sim(u, v) - result) > 0.01) {
#pragma omp critical
                    cout << u << ", " << v << "," << ts.sim(u, v) << "," << result << endl;
                }
            }
            delete[] ansVal;
        };
        auto tmp_end = high_resolution_clock::now();
        auto clock_end = clock();
        cout << "total query cpu time:" << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s" << endl;
        cout << "query time:" << duration_cast<microseconds>(tmp_end - tmp_start).count() / pow(10, 6) << " s\n";
        cout << "max err:" << max_err << endl;
    } else {
        auto tmp_start = high_resolution_clock::now();
        auto clock_start = clock();

#pragma omp parallel
        {
            auto *ansVal = new double[n];

#pragma omp for schedule(dynamic, 100)
            for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
                auto u = sample_pairs[pair_i].first;
                auto v = sample_pairs[pair_i].second;
                auto result = i3.queryOne(u, v, ansVal);
            }
            delete[] ansVal;
        };
        auto tmp_end = high_resolution_clock::now();
        auto clock_end = clock();
        cout << "total query cpu time:" << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s"
             << endl;
        cout << "query time:" << duration_cast<microseconds>(tmp_end - tmp_start).count() / pow(10, 6) << " s\n";
    }
}