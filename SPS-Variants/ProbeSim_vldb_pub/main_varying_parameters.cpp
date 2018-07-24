//
// Created by yche on 7/24/18.
//

#include <chrono>

#include <ground_truth/random_pair_generator.h>
#include <ground_truth/simrank.h>

#include "Graph.h"
#include "SimStruct.h"

using namespace boost;

int main(int argc, char *argv[]) {
    // 1st: load graph
    string file_name = argv[1];
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);

    double c = atof(argv[4]);
    double eps = atof(argv[5]);
    double delta = atof(argv[6]);

    string file_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + file_name + ".txt";

    // 2nd: init gt, sample pairs
#ifdef GROUND_TRUTH
    GraphYche g_gt(file_path);
    TruthSim ts(file_name, g_gt, c, 0.01);
    auto max_err = 0.0;
#endif
    auto sample_pairs = read_sample_pairs(file_name, pair_num, round_i);
    auto start = std::chrono::high_resolution_clock::now();
    auto clock_start = clock();

    SimStruct simStruct = SimStruct(file_path, eps);
    // 3rd: querying pairs
#pragma omp parallel
    {
        SimStruct probe_sim(simStruct, eps);
        auto *resultList = new double[probe_sim.g.n];
#ifdef GROUND_TRUTH
#pragma omp for reduction(max:max_err) schedule(dynamic, 100)
#else
#pragma omp for schedule(dynamic, 100)
#endif
        for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
            auto u = sample_pairs[pair_i].first;
            auto v = sample_pairs[pair_i].second;
#ifdef GROUND_TRUTH
            auto res = probe_sim.batch_for_single_pair(u, v, resultList);
            max_err = max(max_err, abs(ts.sim(u, v) - res));
            if (abs(ts.sim(u, v) - res) > eps) {
#pragma omp critical
                cout << u << ", " << v << "," << ts.sim(u, v) << "," << res << endl;
            }
#else
            probe_sim.batch_for_single_pair(u, v, resultList);
#endif
        }
        delete[]resultList;
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
