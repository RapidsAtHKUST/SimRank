//
// Created by yche on 3/20/18.
//
#include <unordered_map>
#include <vector>
#include <iostream>
#include "../util/pretty_print.h"

using namespace std;

int main() {
    unordered_map<int, vector<int>> dict;
    dict.emplace(1, vector<int>(3));
    dict.emplace(2, vector<int>(4));
    vector<pair<int, vector<int>>> my_vec;
    for (auto &ref: dict) {
        my_vec.emplace_back(std::move(ref));
    }
    cout << dict << endl;
    cout << my_vec << endl;
}