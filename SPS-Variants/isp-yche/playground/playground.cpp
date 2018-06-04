//
// Created by yche on 12/13/17.
//

#include <cmath>

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

void PlayIterAdvance() {
    auto my_vec = vector<int>{0, 1, 2, 5};
    auto iter_beg = begin(my_vec);
    auto iter_end = end(my_vec);
    cout << "size:" << std::distance(iter_beg, iter_end) << endl;

    for (auto i = 0; i < my_vec.size(); i++) {
        auto iter = begin(my_vec);
        std::advance(iter, i);
        cout << *iter << endl;
    }
}

int main() {
//    PlaySpareHashMap();
    double eps = 0.01;
    double c = 0.6;
    cout << static_cast<int>((log(eps) / log(c))) << endl;
}