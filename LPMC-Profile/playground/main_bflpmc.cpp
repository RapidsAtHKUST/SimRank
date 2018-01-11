//
// Created by yche on 12/24/17.
//
#include <cstdlib>

#include <chrono>
#include <iostream>

#include <boost/program_options.hpp>

#include "../yche_refactor/bflpmc_yche.h"
#include "../yche_refactor/simrank.h"

void test_BFLPMC(string data_name, double c, double epsilon, double delta, int x, int y) {
    string path = get_edge_list_path(data_name);
    GraphYche g(path);

    NodePair q{x, y};
    BFLPMC bflpmc(data_name, g, c, epsilon, delta);

    TruthSim ts(data_name, g, c, epsilon);
    auto max_err = 0.0;
    auto min_err = 1.0;
    auto failure_count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for (auto i = 0; i < 2000; i++) {
        double result = bflpmc.query_one2one(q);
        auto cur_err = abs(result - ts.sim(x, y));
        max_err = max(max_err, cur_err);
        min_err = min(min_err, cur_err);
        if (cur_err > 0.01) {
            cout << result << " ," << ts.sim(x, y) << endl;
            failure_count++;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    cout << "failure count:" << failure_count << endl;
    cout << "max err:" << max_err << endl;
    cout << "min err:" << min_err << endl;
}

int main(int argc, char *argv[]) {
    string data_name(argv[1]);
    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;

    int x = atoi(argv[2]), y = atoi(argv[3]);
    test_BFLPMC(data_name, c, epsilon, delta, x, y);
}