//
// Created by yche on 7/24/18.
//

#include <chrono>

#include <ground_truth/random_pair_generator.h>
#include <ground_truth/simrank.h>

#include "util/log.h"
#include "util/timer.h"

#include "Graph.h"
#include "SimStruct.h"


using namespace boost;

int k = 200;

int main(int argc, char *argv[]) {
    // 1st: load graph
    string file_name = argv[1];

    double c = 0.6;
    double eps = 0.01;
    double delta = 0.01;

    string file_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + file_name + ".txt";

    Timer timer;
    GraphYche g_gt(file_path);
    log_info("Loading Time: %.9lfs", timer.elapsed_and_reset());

    log_info("SimStruct Time: %.9lfs", timer.elapsed_and_reset());

    SimStruct simStructGlobal = SimStruct(file_path, c, eps, delta);
    // 3rd: querying pairs
    bool need_ground_truth = g_gt.n < 10000;
    if (need_ground_truth) {
        TruthSim ts(file_name, g_gt, c, 0.01);
        auto max_err = 0.0;
        auto avg_err = 0.0;
        timer.reset();
#pragma omp parallel
        {
            SimStruct simStruct(simStructGlobal, eps);
#pragma omp for schedule(dynamic, 1) reduction(max: max_err) reduction(+:avg_err)
            for (auto u = 0; u < g_gt.n; u++) {

                auto res = simStruct.batch_single_source(u);
                for (auto v = 0; v < g_gt.n; v++) {
                    auto err = abs(ts.sim(u, v) - res[v]);
                    max_err = max(max_err, err);
                    avg_err += err;
                }
            }

        }
        log_info("Computation Time: %.9lfs", timer.elapsed_and_reset());
        log_info("Max Error: %.9lf", max_err);
        log_info("Avg Error: %.9lf", avg_err / g_gt.n / g_gt.n);
    } else {
        SimStruct simStruct = simStructGlobal;
        timer.reset();
#pragma omp parallel for schedule(dynamic, 1)
        for (auto u = 0; u < g_gt.n; u++) {
            auto res = simStruct.batch_single_source(u);
        }
        log_info("Computation Time: %.9lfs", timer.elapsed_and_reset());
    }
    return 0;
}
