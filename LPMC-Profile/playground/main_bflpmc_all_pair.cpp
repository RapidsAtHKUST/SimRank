//
// Created by yche on 12/25/17.
//
#include <cstdlib>

#include <chrono>
#include <iostream>

#include <boost/program_options.hpp>

#include "../yche_refactor/bflpmc_yche.h"

void test_BFLPMC(string data_name, double c, double epsilon, double delta) {
    // test_readsrq(data_name,c,epsilon,R_prime,R,t);
    // test the max heap functionality
    string path = get_edge_list_path(data_name);
    GraphYche g(path);
    size_t n = static_cast<size_t>(g.n);

    cout << n << endl;
    auto start = std::chrono::high_resolution_clock::now();
    auto bflpmc = BFLPMC(data_name, g, c, epsilon, delta);

#pragma omp parallel
    {
        auto local_bflpmc = bflpmc;
#pragma omp for schedule(dynamic, 1)
//        for (auto i = 0u; i < 100; i++) {
        for (auto i = 0u; i < n; i++) {
            for (auto j = i; j < n; j++) {
                auto q = pair<uint32_t, uint32_t>(i, j);
                local_bflpmc.query_one2one(q);
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

//    int x = atoi(argv[2]), y = atoi(argv[3]);
    test_BFLPMC(data_name, c, epsilon, delta);
}