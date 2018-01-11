//
// Created by yche on 12/25/17.
//
#include <cstdlib>

#include <chrono>
#include <iostream>

#include <boost/program_options.hpp>

#include "../yche_refactor/bflpmc_yche.h"
#include "../yche_refactor/simrank.h"

void test_BFLPMC(string data_name, double c, double epsilon, double delta) {
    // init graph
    string path = get_edge_list_path(data_name);
    GraphYche g(path);
    size_t n = static_cast<size_t>(g.n);
    cout << n << endl;

    // init bflpmc object
    auto bflpmc = BFLPMC(data_name, g, c, epsilon, delta);

#ifdef GROUND_TRUTH
    TruthSim ts(data_name, g, c, epsilon);
    auto max_err = 0.0;
#endif

#ifdef GROUND_TRUTH_STATISTICS
    auto above_eps_count = 0;
    auto above_point1_count = 0;
    auto above_point2_count = 0;
    auto above_point3_count = 0;
    auto above_point4_count = 0;
    auto above_point5_count = 0;
    auto above_point6_count = 0;
    for (auto i = 0u; i < n; i++) {
        for (auto j = i; j < n; j++) {
            if (ts.sim(i, j) > 0.01) { above_eps_count++; }
            if (ts.sim(i, j) > 0.1) { above_point1_count++; }
            if (ts.sim(i, j) > 0.2) { above_point2_count++; }
            if (ts.sim(i, j) > 0.3) { above_point3_count++; }
            if (ts.sim(i, j) > 0.4) { above_point4_count++; }
            if (ts.sim(i, j) > 0.5) { above_point5_count++; }
            if (ts.sim(i, j) > 0.6) { above_point6_count++; }
        }
    }
    cout << "above eps:" << above_eps_count << endl;
    cout << "above 0.1 :" << above_point1_count << endl;
    cout << "above 0.2 :" << above_point2_count << endl;
    cout << "above 0.3 :" << above_point3_count << endl;
    cout << "above 0.4 :" << above_point4_count << endl;
    cout << "above 0.5 :" << above_point5_count << endl;
    cout << "above 0.6 :" << above_point6_count << endl;
#endif

    auto start = std::chrono::high_resolution_clock::now();
#pragma omp parallel
    {
        auto local_bflpmc = bflpmc;

#ifdef GROUND_TRUTH
#pragma omp for reduction(max:max_err) schedule(dynamic, 1)
#else
#pragma omp for schedule(dynamic, 1)
#endif

#if defined(ALL_PAIR)
        for (auto i = 0u; i < n; i++) {
            for (auto j = i; j < n; j++) {
#else
                for (auto i = 0u; i < 1000; i++) {
                    for (auto j = i; j < 1000; j++) {
#endif
                auto q = pair<uint32_t, uint32_t>(i, j);
#ifdef GROUND_TRUTH
                auto res = local_bflpmc.query_one2one(q);
                max_err = max(max_err, abs(ts.sim(q.first, q.second) - res));
                if (abs(ts.sim(q.first, q.second) - res) > 0.01) {
#pragma omp critical
                    cout << i << "," << j << "," << ts.sim(q.first, q.second) << "," << res << endl;
                }
#else
                local_bflpmc.query_one2one(q);
#endif
            }
        }

    };
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
#ifdef GROUND_TRUTH
    cout << "max err:" << max_err <<
         endl;
#endif
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}

int main(int args, char *argv[]) {
    string data_name(argv[1]);
    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;

    test_BFLPMC(data_name, c, epsilon, delta);
}