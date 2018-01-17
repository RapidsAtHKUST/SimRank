//
// Created by yche on 12/25/17.
//
#include <cstdlib>

#include <chrono>
#include <iostream>

#include <boost/program_options.hpp>

#include "../../yche_refactor/bflpmc_yche.h"
#include "../../yche_refactor/simrank.h"
#include "../../util/random_pair_generator.h"
#include "../../playground/pretty_print.h"

void test_BFLPMC(string data_name, double c, double epsilon, double delta, int pair_num, int round) {
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

    auto sample_pairs = read_sample_pairs(data_name, pair_num, round);
    auto clock_start = clock();
    auto start = std::chrono::high_resolution_clock::now();
#pragma omp parallel
    {
        auto local_bflpmc = bflpmc;
//        auto local_bflpmc = BFLPMC(data_name, g, c, epsilon, delta);

#ifdef GROUND_TRUTH
#pragma omp for reduction(max:max_err) schedule(dynamic, 100)
#else
#pragma omp for schedule(dynamic, 100)
#endif
        for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
            auto q = pair<uint32_t, uint32_t>(sample_pairs[pair_i].first, sample_pairs[pair_i].second);
#ifdef GROUND_TRUTH
            auto res = local_bflpmc.query_one2one(q);
            max_err = max(max_err, abs(ts.sim(q.first, q.second) - res));
            if (abs(ts.sim(q.first, q.second) - res) > 0.01) {
#pragma omp critical
                cout << sample_pairs[pair_i].first << "," << sample_pairs[pair_i].second << ","
                << ts.sim(q.first, q.second) << "," << res << endl;
            }
#else
            local_bflpmc.query_one2one(q);
#endif
        }
    };

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    auto clock_end = clock();
    cout << "total query cpu time:" << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s" << endl;
#ifdef GROUND_TRUTH
    cout << "max err:" << max_err << endl;
#endif
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}

int main(int args, char *argv[]) {
    string data_name(argv[1]);
    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;


    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
    test_BFLPMC(data_name, c, epsilon, delta, pair_num, round_i);
}