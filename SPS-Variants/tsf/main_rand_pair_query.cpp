//
// Created by yche on 1/8/18.
//
#include <iostream>
#include <chrono>

#include "yche_tsf.h"
#include "ground_truth/graph_yche.h"
#include "ground_truth/simrank.h"
#include "ground_truth/stat.h"
#include "input_output.h"
#include "ground_truth/random_pair_generator.h"

using namespace std;
using namespace std::chrono;


int main(int argc, char *argv[]) {
    auto decayFactor = 0.6;
    auto stepNum = 11;
    auto sampleNum = 100;
    auto sampleQueryNum = 43;

    // 1st: load graph
    string file_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + string(argv[1]) + ".txt";
    string file_name = string(argv[1]);
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
    double c = 0.6;
    int max_iter = 9;
    double filter_threshold = 0.0001;

    auto tmp_start = std::chrono::high_resolution_clock::now();
    vector<int> graph_src_vec;
    vector<int> graph_dst_vec;

    InitGraph(file_path, graph_src_vec, graph_dst_vec);

    auto yche_tfs = YcheTSF(sampleNum, sampleQueryNum, stepNum, decayFactor, graph_src_vec, graph_dst_vec);
    auto tmp_end = std::chrono::high_resolution_clock::now();

    cout << "finish input graph " << duration_cast<microseconds>(tmp_end - tmp_start).count() << " us\n";
    cout << "mem size:" << getValue() << endl;
#ifdef GROUND_TRUTH
    GraphYche g_gt(file_path);
    TruthSim ts(string(argv[1]), g_gt, c, 0.01);
    auto max_err = 0.0;
#endif

    auto sample_pairs = read_sample_pairs(file_name, pair_num, round_i);
    auto start = std::chrono::high_resolution_clock::now();
    auto clock_start = clock();

#ifdef GROUND_TRUTH
#pragma omp parallel for reduction(max:max_err) schedule(dynamic, 100)
#else
#pragma omp parallel for schedule(dynamic, 100)
#endif
    for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
        auto u = sample_pairs[pair_i].first;
        auto v = sample_pairs[pair_i].second;
#ifdef GROUND_TRUTH
        auto res = yche_tfs.querySinglePair(u, v);
        max_err = max(max_err, abs(ts.sim(u, v) - res));
        if (abs(ts.sim(u, v) - res) > 0.01) {
#pragma omp critical
            cout << u << ", " << v << "," << ts.sim(u, v) << "," << res << endl;
        }
#else
        yche_tfs.querySinglePair(u, v);
#endif
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto clock_end = clock();
    cout << "total query cpu time:" << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s" << endl;
    std::chrono::duration<double> elapsed = end - start;
#ifdef GROUND_TRUTH
    cout << "max err:" << max_err << endl;
#endif
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}