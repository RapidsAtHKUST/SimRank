#include <chrono>
#include <set>

#include <ground_truth/random_pair_generator.h>
#include <ground_truth/simrank.h>

#include "Graph.h"
#include "SimStruct.h"

using namespace boost;

int main(int argc, char *argv[]) {
    string file_name = argv[1];
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
    int k = atoi(argv[4]);
    int truth_graph_size = 10000;

    double c = 0.6;
    double eps = 0.01;
    double delta = 0.01;

    string file_path = "/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC/build/edge_list/" + file_name + ".txt";

    SimStruct simStruct = SimStruct(file_path, eps);
    SimStruct probe_sim(simStruct, eps);
    auto *resultList = new double[probe_sim.g.n];

    TruthSim *ts;
    if (n < probe_sim.g.n) {
        GraphYche g_gt(file_path);
        ts = new TruthSim(file_name, g_gt, c, 0.01);
    }

    return 0;
}
