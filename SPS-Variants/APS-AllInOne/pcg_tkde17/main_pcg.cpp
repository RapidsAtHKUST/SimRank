//
// Created by yche on 9/17/19.
//
#include <ground_truth/simrank.h>
#include "tkde17_sim.h"

int main(int argc, char *argv[]) {
    auto data_name = string(argv[1]);
    double c = 0.6;
    double epsilon = 0.01;

    auto full_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + data_name + ".txt";
    GraphYche g_gt(full_path);

    if (g_gt.n < 10000) {
        TruthSim ts(data_name, g_gt, c, epsilon);

        DirectedG g;
        load_graph(get_edge_list_path(data_name), g);
        cout << "computing " << data_name << " " << c << epsilon << endl;
        LinearSystemSim lys(data_name, g, c, epsilon);
        auto &res_matrix = lys.sim;
        double max_err = 0;
        double avg_err = 0;
#pragma omp parallel for reduction(max:max_err) reduction(+:avg_err)
        for (auto u = 0; u < g_gt.n; u++) {
            for (auto v = u; v < g_gt.n; v++) {
                double err = abs(ts.sim(u, v) - res_matrix(u, v));
                max_err = max(max_err, err);
                avg_err += err;
            }
        }
        log_info("Max Error: %.9lf", max_err);
        log_info("Avg Error: %.9lf", avg_err / g_gt.n / g_gt.n);
    } else {
        log_info("Not Supported... Need Large Memory");
    }
    return 0;
}