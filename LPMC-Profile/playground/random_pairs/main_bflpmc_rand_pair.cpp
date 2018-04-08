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

#ifdef GROUND_TRUTH
int k = 200;
#endif

double eps = 0.01;

void test_BFLPMC(string data_name, double c, double epsilon, double delta, int pair_num, int round) {
    // init graph
    string path = get_edge_list_path(data_name);
    GraphYche g(path);
    auto n = static_cast<size_t>(g.n);
    cout << n << endl;

    // init bflpmc object
    auto bflpmc = BFLPMC(data_name, g, c, epsilon, delta);
    auto sample_pairs = read_sample_pairs(data_name, pair_num, round);

#ifdef GROUND_TRUTH
    vector<float> sim_val_arr(pair_num);
    TruthSim ts(data_name, g, c, epsilon);
    auto max_err = 0.0;


    for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
        int i, j;
        std::tie(i, j) = sample_pairs[pair_i];
        sim_val_arr[pair_i] = ts.sim(i, j);
    }
    vector<int> idx_arr(sim_val_arr.size());
    for (auto i = 0; i < idx_arr.size(); i++) { idx_arr[i] = i; }
    std::sort(std::begin(idx_arr), std::end(idx_arr),
              [&sim_val_arr](int l, int r) { return sim_val_arr[l] > sim_val_arr[r]; });
    vector<float> sim_val_computed(sim_val_arr.size());
#endif

    auto clock_start = clock();
    auto start = std::chrono::high_resolution_clock::now();
#pragma omp parallel
    {
        auto local_bflpmc = bflpmc;

#ifdef GROUND_TRUTH
#pragma omp for reduction(max:max_err) schedule(dynamic, 100)
#else
#pragma omp for schedule(dynamic, 100)
#endif
        for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
            auto q = pair<uint32_t, uint32_t>(sample_pairs[pair_i].first, sample_pairs[pair_i].second);
#ifdef GROUND_TRUTH
            auto res = local_bflpmc.query_one2one(q);
            sim_val_computed[pair_i] = static_cast<float>(res);
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
    vector<int> idx_arr_our_sol(sim_val_arr.size());
    for (auto i = 0; i < idx_arr_our_sol.size(); i++) { idx_arr_our_sol[i] = i; }
    std::sort(std::begin(idx_arr_our_sol), std::end(idx_arr_our_sol),
              [&sim_val_computed](int l, int r) { return sim_val_computed[l] > sim_val_computed[r]; });
    std::sort(std::begin(idx_arr), std::begin(idx_arr) + k);
    std::sort(std::begin(idx_arr_our_sol), std::begin(idx_arr_our_sol) + k);

    vector<int> intersection_arr;
    std::set_intersection(std::begin(idx_arr), std::begin(idx_arr) + k, std::begin(idx_arr_our_sol),
                          std::begin(idx_arr_our_sol) + k, back_inserter(intersection_arr));
    cout << "precision #:" << intersection_arr.size() << "/" << k << endl;

#endif
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}

int main(int argc, char *argv[]) {
    string data_name(argv[1]);
    double c = 0.6;
    double delta = 0.01;

    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
#ifdef GROUND_TRUTH
    if (argc >= 5 && string(argv[4]) != string(">>") && string(argv[4]) != string(">")) {
        k = atoi(argv[4]);
        eps = atof(argv[5]);
    }
#endif
    double epsilon = eps;

    test_BFLPMC(data_name, c, epsilon, delta, pair_num, round_i);
}