//
// Created by yche on 1/7/18.
//
#include <iostream>
#include <chrono>

#include "ISP.h"
#include "ground_truth/graph_yche.h"
#include "ground_truth/simrank.h"

using namespace std;
using namespace std::chrono;


int main(int argc, char *argv[]) {
    // 1st: load graph
    string file_path = "./datasets/edge_list/" + string(argv[1]) + ".txt";
    double c = 0.6;
    int max_iter = 9;
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

    tmp_start = std::chrono::high_resolution_clock::now();
#ifdef GROUND_TRUTH
#pragma omp parallel for reduction(max:max_err) schedule(dynamic, 1)
#else
#pragma omp parallel for schedule(dynamic, 1)
#endif
    for (auto u = 0; u < 1000; u++) {
        for (auto v = u; v < 1000; v++) {
#ifdef GROUND_TRUTH
            auto res = my_isp.ComputeSim(u, v, c, max_iter, filter_threshold);
            max_err = max(max_err, abs(ts.sim(u, v) - res));
            if (abs(ts.sim(u, v) - res) > 0.01) {
#pragma omp critical
                cout << u << ", " << v << "," << ts.sim(u, v) << "," << res << endl;
            }
#else
            my_isp.ComputeSim(u, v, c, max_iter, filter_threshold);
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