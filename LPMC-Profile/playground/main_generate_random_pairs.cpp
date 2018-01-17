//
// Created by yche on 1/16/18.
//


#include <iostream>

#include "../util/random_pair_generator.h"
#include "pretty_print.h"

using namespace std;

int main(int argc, char *argv[]) {
    string graph_name = argv[1];
    int pair_num = atoi(argv[2]);
    int round_num = atoi(argv[3]);
    if (argc == 4) {
        generate_sample_pairs(graph_name, pair_num, round_num);
    } else {
        int round_i = atoi(argv[3]);
        cout << read_sample_pairs(graph_name, pair_num, round_i) << endl;
    }
}