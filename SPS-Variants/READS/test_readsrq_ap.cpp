//
// Created by yche on 12/17/17.
//

#include <cmath>

#include <iostream>

#include "algorithm/reads.h"

#include "util/sample_num.h"
#include "algorithm/readsrq.h"

#include "ground_truth/graph_yche.h"
#include "ground_truth/simrank.h"

using namespace std;
using namespace std::chrono;

// usage and example:
// g++ reads*.h reads*.cpp timer.h -O3 -w -std=c++11 test.cpp -I . && ./a.out hp.data 3133
int main(int argc, char **argv) {
    FILE *log_f = nullptr;
    if (argc >= 3) {
        log_f = fopen(argv[2], "a+");
        log_set_fp(log_f);
    }
    // 1st: eps, delta and c for computing sample number
    double eps = 0.01;
    double delta = 0.01;
    double c = 0.6;
    int r = 100;
    int rq = compute_reads_rq_num(eps, delta, c, r);
    {
        stringstream ss;
        ss << "sample num:" << r << ", on-line rand-walk:" << rq;
        log_info("%s", ss.str().c_str());
    }

    // 2nd: t(max length), n
    int t = 10;
    auto data_name = string(argv[1]);
    auto full_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + data_name + ".txt";
    GraphYche g_gt(full_path);
    int n = g_gt.n;
    log_info("vertex num: %d", n);

    // 3rd: construct index
    Timer timer;
    ClockTimer clock_timer;
    readsrq i3(data_name, n, r, rq, c, t);
    log_info("Indexing Time: %.9lfs", timer.elapsed());

    log_info("Initial Memory Consumption: %d KB", getValue());
    // 4th: query
    if (n < 10000) {
        double max_err = 0;
        double avg_err = 0;
        TruthSim ts(data_name, g_gt, c, 0.01);
        log_info("After TruthSim Memory Consumption: %d KB", getValue());

#pragma omp parallel
        {
            auto *ansVal = new double[n];
#pragma omp for reduction(max:max_err) reduction(+:avg_err) schedule(dynamic, 1)
            for (auto u = 0; u < g_gt.n; u++) {
                i3.queryAll(u, ansVal);

                for (auto v = 0; v < g_gt.n; v++) {
                    auto result = ansVal[v];
                    auto err = abs(ts.sim(u, v) - result);
                    max_err = max(max_err, err);
                    avg_err += err;
                }
            }
            delete[] ansVal;
        }
        log_info("Max Error: %.9lf", max_err);
        log_info("Avg Error: %.9lf", avg_err / g_gt.n / g_gt.n);
    } else {
#pragma omp parallel
        {
            auto *ansVal = new double[n];
#pragma omp for schedule(dynamic, 1)
            for (auto u = 0; u < g_gt.n; u++) {
                i3.queryAll(u, ansVal);
            }
            delete[] ansVal;
        }
    }
    log_info("Clock Time: %.9lfs", clock_timer.elapsed_and_reset());
    log_info("Computation Time: %.9lfs", timer.elapsed_and_reset());
    log_info("Final Memory Consumption: %d KB", getValue());

    if (log_f != nullptr) {
        log_info("Flush File and Close...");
        fflush(log_f);
        fclose(log_f);
    }
}