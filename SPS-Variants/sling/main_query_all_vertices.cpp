//
// Created by yche on 12/20/17.
//

#include <iostream>

#include "ground_truth/simrank.h"

#include "sling.h"
#include "ground_truth/stat.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
    double eps_d = 0.002;
    double theta = 0.00029;
    double c = 0.6;

    // 1st: load graph
    Graph g;
    string file_name = argv[1];
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

    // 3rd: querying pairs
    tmp_start = std::chrono::high_resolution_clock::now();
#ifdef GROUND_TRUTH
#pragma omp parallel for reduction(max:max_err) schedule(dynamic, 1)
#else
#pragma omp parallel for schedule(dynamic, 1)
#endif
#ifdef ALL_PAIR
    for (auto u = 0; u < sling_algo.g->n; u++) {
        for (auto v = u; v < sling_algo.g->n; v++) {
#else
            for (auto u = 0; u < 1000; u++) {
                for (auto v = u; v < 1000; v++) {
#endif
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

    }
    tmp_end = std::chrono::high_resolution_clock::now();

#ifdef GROUND_TRUTH
    cout << "max err:" << max_err << endl;
#endif
    cout << "query time:" << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6)) << " s\n";
}