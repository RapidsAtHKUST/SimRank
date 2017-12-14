//
// Created by yche on 12/13/17.
//

#include <iostream>

#include <sparsepp/spp.h>

#include "../pretty_print.h"

#include <boost/graph/adjacency_list.hpp>

using spp::sparse_hash_map;
using namespace std;


void PlaySpareHashMap() {
    auto my_dict = sparse_hash_map<uint32_t, int>();
    my_dict[2] = 10;
    cout << my_dict[1] << endl;
    cout << my_dict[2] << endl;
    for (auto &my_pair: my_dict) {
        cout << my_pair << endl;
    }
}

int main() {
    PlaySpareHashMap();
}