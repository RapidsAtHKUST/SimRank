//
// Created by yche on 1/8/18.
//

#include <chrono>
#include <util/timer.h>

#include "util/graph_yche.h"
#include "ground_truth/simrank.h"

#include "cloud_walker.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
    FILE *log_f = nullptr;
    if (argc >= 3) {
        log_f = fopen(argv[2], "a+");
        log_set_fp(log_f);
    }
    double c = 0.6;
    int L = 3;
    int R = 100;

    int T = 10;
    int R_prime = 10000;

    DirectedG g;
    auto data_name = string(argv[1]);
    load_graph(get_edge_list_path(data_name), g);
    srand(static_cast<unsigned int>(time(nullptr))); // random number generator

    // 1st: indexing
    Timer timer;
    CloudWalker cw(&g, data_name, c, T, L, R, R_prime);
    log_info("constructing time: %.9lfs", timer.elapsed_and_reset());

    // 2nd: query
#ifdef GROUND_TRUTH
    string path = get_edge_list_path(data_name);
    GraphYche g_gt(path);
    TruthSim ts(string(argv[1]), g_gt, c, 0.01);
    auto max_err = 0.0;
#endif

    timer.reset();
#pragma omp parallel
    {
        // allocate memory in advance
        MatrixXd pos_dist_v(T + 1, cw.n);
        MatrixXd pos_dist_u(T + 1, cw.n);
#ifdef GROUND_TRUTH
#pragma omp for reduction(max:max_err) schedule(dynamic, 1)
#else
#pragma omp for schedule(dynamic, 1)
#endif
        for (auto u = 0; u < 100; u++) {
            for (auto v = u; v < 100; v++) {
#ifdef GROUND_TRUTH
                auto res = cw.mcsp(u, v, pos_dist_u, pos_dist_v);

                max_err = max(max_err, abs(ts.sim(u, v) - res));
                if (abs(ts.sim(u, v) - res) > 0.01) {
#pragma omp critical
                    cout << u << ", " << v << "," << ts.sim(u, v) << "," << res << endl;
                }
#else
                cw.mcsp(u, v, pos_dist_u, pos_dist_v);
#endif
            }
        }
    };
    auto tmp_end = std::chrono::high_resolution_clock::now();

#ifdef GROUND_TRUTH
    cout << "max err:" << max_err << endl;
#endif
    log_info("query time: %.9lfs", timer.elapsed());
    if (log_f != nullptr) {
        log_info("Flush File and Close...");
        fflush(log_f);
        fclose(log_f);
    }
}