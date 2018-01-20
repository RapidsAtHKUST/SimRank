//
// Created by yche on 1/8/18.
//

#include <cmath>

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
    // 1st: load graph
    string file_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + string(argv[1]) + ".txt";
    string file_name = string(argv[1]);
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);

    // 2nd: varying parameters
    auto decayFactor = atof(argv[4]);
    auto stepNum = 11;    // fixed, it is an approximate value also
    auto sampleNum = 100; // fixed

    // three varying parameters
    double eps = atof(argv[5]);
    double delta = atof(argv[6]);

    double c = decayFactor;
    double b = min(1.0, c / (1 - c));
    auto sampleQueryNum = static_cast<int>(ceil(log(delta / 2) / (-2) / (pow(eps, 2)) * (pow(b - c, 2)) / sampleNum));
    cout << sampleQueryNum << endl;

    // tsf algorithm .............................................
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
    auto failure_count = 0;
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
        if (abs(ts.sim(u, v) - res) > eps) {
#pragma omp critical
        {
            cout << u << ", " << v << "," << ts.sim(u, v) << "," << res << endl;
            failure_count++;
        }
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
    cout << "failure count:" << failure_count << endl;
    cout << "max err:" << max_err << endl;
#endif
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}