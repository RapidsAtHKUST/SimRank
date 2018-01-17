//
// Created by yche on 12/20/17.
//

#include <iostream>

#include "ground_truth/simrank.h"

#include "sling.h"
#include "ground_truth/stat.h"
#include "ground_truth/random_pair_generator.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
    double eps_d = 0.002;
    double theta = 0.00029;
    double c = 0.6;

    // 1st: load graph
    Graph g;
    string file_name = argv[1];
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
    string file_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + file_name + ".txt";
    g.inputGraph(file_path);

    // 2nd: construct sling algorithm, indexing
    auto tmp_start = std::chrono::high_resolution_clock::now();
    Sling sling_algo(&g, file_name, c, eps_d, theta);
    auto tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish input graph and construct indexing: " << duration_cast<milliseconds>(tmp_end - tmp_start).count()
         << " ms\n";

#ifdef GROUND_TRUTH
    GraphYche g_gt(file_path);
    TruthSim ts(file_name, g_gt, c, 0.01);
    auto max_err = 0.0;
#endif
    auto sample_pairs = read_sample_pairs(file_name, pair_num, round_i);
    auto start = std::chrono::high_resolution_clock::now();
    auto clock_start = clock();

    // 3rd: querying pairs
#ifdef GROUND_TRUTH
#pragma omp parallel for reduction(max:max_err) schedule(dynamic, 100)
#else
#pragma omp parallel for schedule(dynamic, 100)
#endif
    for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
        auto u = sample_pairs[pair_i].first;
        auto v = sample_pairs[pair_i].second;
#ifdef GROUND_TRUTH
        auto res = sling_algo.simrank(u, v);
        max_err = max(max_err, abs(ts.sim(u, v) - res));
        if (abs(ts.sim(u, v) - res) > 0.01) {
#pragma omp critical
            cout << u << ", " << v << "," << ts.sim(u, v) << "," << res << endl;
        }
#else
        sling_algo.simrank(u, v);
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