//
// Created by yche on 12/20/17.
//

#include <iostream>

#include "ground_truth/simrank.h"
#include "ground_truth/stat.h"

#include "util/timer.h"
#include "util/log.h"

#include "sling.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
    // eps = 0.01
    double eps_d = 0.002;
    double theta = 0.00029;
    double c = 0.6;

    // 1st: load graph
    double indexing_time;
    double comp_time;
    Timer timer;
    Graph g;
    string file_name = argv[1];
    string file_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + file_name + ".txt";
    g.inputGraph(file_path);
    log_info("Loading Time: %.9lfs", timer.elapsed_and_reset());

    // 2nd: construct sling algorithm, indexing
    Sling sling_algo(&g, file_name, c, eps_d, theta);
    indexing_time = timer.elapsed();
    log_info("Indexing Time: %.9lfs", timer.elapsed_and_reset());

    bool need_ground_truth = g.n < 10000;
    if (need_ground_truth) {
        GraphYche g_gt(file_path);
        TruthSim ts(file_name, g_gt, c, 0.01);
        auto max_err = 0.0;
        auto avg_err = 0.0;
        timer.reset();
#pragma omp parallel for schedule(dynamic, 1) reduction(max: max_err) reduction(+:avg_err)
        for (auto u = 0; u < g.n; u++) {
            auto res = sling_algo.simrank(u);
            for (auto v = 0; v < g.n; v++) {
                auto err = abs(ts.sim(u, v) - res[v]);
                max_err = max(max_err, err);
                avg_err += err;
            }
        }
        comp_time = timer.elapsed();
        log_info("Computation Time: %.9lfs", timer.elapsed_and_reset());
        log_info("Max Error: %.9lf", max_err);
        log_info("Avg Error: %.9lf", avg_err / g.n / g.n);
    } else {
        timer.reset();
#pragma omp parallel for schedule(dynamic, 1)
        for (auto u = 0; u < g.n; u++) {
            auto res = sling_algo.simrank(u);
        }
        comp_time = timer.elapsed();
        log_info("Computation Time: %.9lfs", timer.elapsed_and_reset());
    }
    log_info("Total Time: %.9lfs", indexing_time + comp_time);
}
