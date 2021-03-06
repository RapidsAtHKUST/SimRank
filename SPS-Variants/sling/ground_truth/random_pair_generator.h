//
// Created by yche on 1/17/18.
//

#ifndef LPMC_OPTIMIZING_RANDOM_PAIR_GENERATOR_H
#define LPMC_OPTIMIZING_RANDOM_PAIR_GENERATOR_H

#include <random>

#include <boost/format.hpp>

#include "util/util.h"
#include "graph_yche.h"
#include "yche_serialization.h"

using namespace std;

const string INPUT_PAIRS_DIR =
        string(DATA_ROOT) + "/input_pairs/";

inline string get_file_path(string g_name, int round_num, int sample_num) {
    exec(string("mkdir -p " + INPUT_PAIRS_DIR).c_str());
    string file_path =
            INPUT_PAIRS_DIR + boost::str(boost::format("RANDOM_PAIRS_%s-%s-%s") % g_name % sample_num % round_num);
    return file_path;
}

void generate_sample_pairs(string graph_name, int pair_num, int round_num);

vector<pair<int, int>> read_sample_pairs(string graph_name, int pair_num, int round);

#endif //LPMC_OPTIMIZING_RANDOM_PAIR_GENERATOR_H
