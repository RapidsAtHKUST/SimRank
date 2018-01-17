//
// Created by yche on 1/17/18.
//
#include <iostream>

#include "random_pair_generator.h"

#include "../playground/pretty_print.h"

void generate_sample_pairs(string graph_name, int pair_num, int round_num) {
    string path = get_edge_list_path(graph_name);
    GraphYche g(path);

    // generate pairs
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> distribution(0, g.n - 1);

    for (auto round_i = 0; round_i < round_num; round_i++) {
        vector<pair<int, int>> sample_pairs;
        for (auto i = 0; i < pair_num; i++) {
            auto u = distribution(gen);
            auto v = distribution(gen);
            sample_pairs.emplace_back(u, v);
        }
#ifdef DEBUG
        cout << sample_pairs << endl;
        cout << get_file_path(graph_name, round_i, pair_num) << endl;
#endif

        string ofs_file_path = get_file_path(graph_name, round_i, pair_num);
        FILE *ofs = fopen(ofs_file_path.c_str(), "wb");
        YcheSerializer serializer;
        serializer.write_vec(ofs, sample_pairs);
        fclose(ofs);
    }
}

vector<pair<int, int>> read_sample_pairs(string graph_name, int pair_num, int round) {
    vector<pair<int, int>> sample_pairs;
    string ifs_file_path = get_file_path(graph_name, round, pair_num);

    FILE *ifs = fopen(ifs_file_path.c_str(), "r");
    YcheSerializer serializer;
    serializer.read_vec(ifs, sample_pairs);
    fclose(ifs);
    return sample_pairs;
};