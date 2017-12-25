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

    size_t n = static_cast<size_t>(g.n);
    NodePair q{x, y};
    BFLPMC bflpmc(data_name, g, c, epsilon, delta);

    double result = bflpmc.query_one2one({x, y});
    cout << format("result of BFLPMC: %s") % result << endl;

    TruthSim ts(data_name, g, c, epsilon);
    cout << format("ground truth: %s") % ts.sim(x, y) << endl;
    cout << format("error: %s") % (ts.sim(q.first, q.second) - result) << endl;

}

int main(int argc, char *argv[]) {
    string data_name(argv[1]);
    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;

    int x = atoi(argv[2]), y = atoi(argv[3]);
    test_BFLPMC(data_name, c, epsilon, delta, x, y);
}