//
// Created by yche on 12/20/17.
//

#include "ground_truth/simrank.h"
#include "util/stat.h"

#include "util/timer.h"
#include "util/log.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
    FILE *log_f = nullptr;
    if (argc >= 3) {
        log_f = fopen(argv[2], "a+");
        log_set_fp(log_f);
    }
    // eps = 0.01
    double c = 0.6;

    // 1st: load graph
    Timer timer;
    string file_name = argv[1];
    string file_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + file_name + ".txt";
    GraphYche g_gt(file_path);
    log_info("Loading Time: %.9lfs", timer.elapsed_and_reset());

    log_info("Initial Memory Consumption: %d KB", getValue());
    bool need_ground_truth = g_gt.n < 10000;
    if (need_ground_truth) {
        TruthSim ts(file_name, g_gt, c, 0.01);
        log_info("After TruthSim Memory Consumption: %d KB", getValue());
    }

    if (log_f != nullptr) {
        log_info("Flush File and Close...");
        fflush(log_f);
        fclose(log_f);
    }
    return 0;
}
