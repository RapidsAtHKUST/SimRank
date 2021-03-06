//
// Created by yche on 1/7/18.
//

#include <cmath>

#include <iostream>
#include <chrono>

#include "ISP.h"
#include "ground_truth/graph_yche.h"
#include "ground_truth/simrank.h"
#include "ground_truth/random_pair_generator.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
    // 1st: load graph
    string file_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + string(argv[1]) + ".txt";
    string file_name = string(argv[1]);
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);

    // init c and eps, which determines max_iter
    double c = atof(argv[4]);
    double eps = atof(argv[5]);
    auto max_iter = static_cast<int>(log(eps) / log(c));

    double filter_threshold = 0.0001;

    auto tmp_start = std::chrono::high_resolution_clock::now();
    ISP my_isp(file_path);
    auto tmp_end = std::chrono::high_resolution_clock::now();

    cout << "finish input graph " << duration_cast<microseconds>(tmp_end - tmp_start).count() << " us\n";

#ifdef GROUND_TRUTH
    GraphYche g_gt(file_path);
    TruthSim ts(string(argv[1]), g_gt, c, 0.01);
    auto max_err = 0.0;
#endif

    auto failure_count = 0;
    auto sample_pairs = read_sample_pairs(file_name, pair_num, round_i);
    auto start = std::chrono::high_resolution_clock::now();
    auto clock_start = clock();

#ifdef GROUND_TRUTH
#pragma omp parallel for reduction(max:max_err) schedule(dynamic, 100)
#else
#pragma omp parallel for schedule(dynamic, 1)
#endif
    for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
        auto u = sample_pairs[pair_i].first;
        auto v = sample_pairs[pair_i].second;
#ifdef GROUND_TRUTH
        auto res = my_isp.ComputeSim(u, v, c, max_iter, filter_threshold);
        max_err = max(max_err, abs(ts.sim(u, v) - res));
        if (abs(ts.sim(u, v) - res) > eps) {
#pragma omp critical
            {
                cout << u << ", " << v << "," << ts.sim(u, v) << "," << res << endl;
                failure_count++;
            };
        }
#else
        my_isp.ComputeSim(u, v, c, max_iter, filter_threshold);
#endif
    }

    tmp_end = std::chrono::high_resolution_clock::now();

    auto end = std::chrono::high_resolution_clock::now();
    auto clock_end = clock();
    cout << "total query cpu time:" << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s" << endl;
    std::chrono::duration<double> elapsed = end - start;
#ifdef GROUND_TRUTH
    cout << "failure count:" << failure_count << endl;
    cout << "max err:" << max_err << endl;
#endif
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}