//
// Created by yche on 12/20/17.
//
#include <cstdlib>
#include <chrono>

#include <iostream>

#include <boost/program_options.hpp>

#include "../yche_refactor/bprw_yche.h"

using namespace std;
using namespace boost::program_options;

void test_bp(string data_name, double c, double epsilon, double delta) {
    string path = get_edge_list_path(data_name);
    GraphYche g(path);
    size_t n = static_cast<size_t>(g.n);

    cout << n << endl;
    auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel
    {
        auto bprw = BackPush(data_name, g, c, epsilon, delta);
#pragma omp for schedule(dynamic, 1)
//        for (auto i = 0u; i < 100; i++) {
        for (auto i = 0u; i < n; i++) {
            for (auto j = i; j < n; j++) {
                auto q = pair<uint32_t, uint32_t>(i, j);
                bprw.query_one2one(q);
            }
        }
    };

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}

int main(int args, char *argv[]) {
    string data_name(argv[1]);
    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;

    test_bp(data_name, c, epsilon, delta);
}