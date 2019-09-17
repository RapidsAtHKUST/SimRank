//
// Created by yche on 1/17/18.
//

#ifndef LPMC_OPTIMIZING_RANDOM_PAIR_GENERATOR_H
#define LPMC_OPTIMIZING_RANDOM_PAIR_GENERATOR_H

#include <random>

#include <boost/format.hpp>

#include "../util/graph_yche.h"
#include "../util/yche_serialization.h"

using namespace std;

const string INPUT_PAIRS_DIR =
        "/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/datasets/input_pairs/";

inline string get_file_path(string g_name, int round_num, int sample_num) {
    return INPUT_PAIRS_DIR + boost::str(boost::format("RANDOM_PAIRS_%s-%s-%s") % g_name % sample_num % round_num);
}

void generate_sample_pairs(string graph_name, int pair_num, int round_num);

vector<pair<int, int>> read_sample_pairs(string graph_name, int pair_num, int round);

#endif //LPMC_OPTIMIZING_RANDOM_PAIR_GENERATOR_H
