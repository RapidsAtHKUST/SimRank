//
// Created by yche on 12/20/17.
//

//
// Created by yche on 11/19/17.
//

#include <iostream>

#include "ground_truth/simrank.h"

#include "sling.h"
#include "graph.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
    // 1st: load graph
    Graph g;
    string file_path = argv[1];
    g.inputGraph(file_path);

    double c = atof(argv[2]);

    auto tmp_start = std::chrono::high_resolution_clock::now();
    Sling sling_algo(&g, c);
    auto tmp_end = std::chrono::high_resolution_clock::now();

    cout << "finish input graph " << duration_cast<microseconds>(tmp_end - tmp_start).count() << " us\n";

#ifdef GROUND_TRUTH
    GraphYche g_gt(file_path);
    TruthSim ts(string(argv[3]), g_gt, c, 0.01);
    auto max_err = 0.0;
#endif

    // build the index
    cout << "indexing..." << endl;

    tmp_start = std::chrono::high_resolution_clock::now();
//    sling_algo.calcD(0.005);
    sling_algo.calcD(0.002);
    tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish calcD " << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6))
         << " s\n";

    tmp_start = std::chrono::high_resolution_clock::now();
//    sling_algo.backward(0.000725);
    sling_algo.backward(0.00029);

    tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish backward " << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6))
         << " s\n";

//    int u = atoi(argv[3]);
//    int v = atoi(argv[4]);

    tmp_start = std::chrono::high_resolution_clock::now();
#ifdef GROUND_TRUTH
#pragma omp parallel for reduction(max:max_err) schedule(dynamic, 1)
#else
#pragma omp parallel for schedule(dynamic, 1)
#endif
//    for (auto u = 0; u < sling_algo.g->n; u++) {
    for (auto u = 0; u < 1000; u++) {
//        for (auto v = u; v < sling_algo.g->n; v++) {
        for (auto v = u; v < 1000; v++) {
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
    cout << "query time:"
         << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6)) << " s\n";
}