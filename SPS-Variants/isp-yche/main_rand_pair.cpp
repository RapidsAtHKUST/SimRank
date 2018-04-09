//
// Created by yche on 1/7/18.
//

#include <cmath>

#include <iostream>
#include <chrono>

#include "ISP.h"
#include "ground_truth/graph_yche.h"
#include "ground_truth/simrank.h"
#include "ground_truth/random_pair_generator.h"

using namespace std;
using namespace std::chrono;

#ifdef GROUND_TRUTH
int k = 200;
#endif

double eps = 0.01;

int main(int argc, char *argv[]) {
    // 1st: load graph
    string file_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + string(argv[1]) + ".txt";
    string file_name = string(argv[1]);
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
#ifdef GROUND_TRUTH
    if (argc >= 5 && string(argv[4]) != string(">>") && string(argv[4]) != string(">")) {
        k = atoi(argv[4]);
        eps = atof(argv[5]);
    }
#endif
    double c = 0.6;

    auto max_iter = static_cast<int>(log(eps) / log(c));
    cout << "iter#:" << max_iter << endl;
    double filter_threshold = 0.0001;

    auto tmp_start = std::chrono::high_resolution_clock::now();
    ISP my_isp(file_path);
    auto tmp_end = std::chrono::high_resolution_clock::now();

    cout << "finish input graph " << duration_cast<microseconds>(tmp_end - tmp_start).count() << " us\n";
    auto sample_pairs = read_sample_pairs(file_name, pair_num, round_i);

#ifdef GROUND_TRUTH
    GraphYche g_gt(file_path);
    TruthSim ts(string(argv[1]), g_gt, c, 0.01);
    auto max_err = 0.0;

    vector<float> sim_val_arr(pair_num);

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

    auto failure_count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    auto clock_start = clock();

#ifdef GROUND_TRUTH
#pragma omp parallel for reduction(max:max_err) schedule(dynamic, 100)
#else
#pragma omp parallel for schedule(dynamic, 100)
#endif
    for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
        auto u = sample_pairs[pair_i].first;
        auto v = sample_pairs[pair_i].second;
#ifdef GROUND_TRUTH
        auto res = my_isp.ComputeSim(u, v, c, max_iter, filter_threshold);
        sim_val_computed[pair_i] = static_cast<float>(res);

        max_err = max(max_err, abs(ts.sim(u, v) - res));
        if (abs(ts.sim(u, v) - res) > 0.01) {
#pragma omp critical
        {
            cout << u << ", " << v << "," << ts.sim(u, v) << "," << res << endl;
            failure_count++;
        };
        }
#else
        my_isp.ComputeSim(u, v, c, max_iter, filter_threshold);
#endif
    }

    tmp_end = std::chrono::high_resolution_clock::now();

    auto end = std::chrono::high_resolution_clock::now();
    auto clock_end = clock();
    cout << "total query cpu time:" << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s" << endl;
    std::chrono::duration<double> elapsed = end - start;
#ifdef GROUND_TRUTH
    cout << "failure count:" << failure_count << endl;
    cout << "max err:" << max_err << endl;

    vector<int> idx_arr_our_sol(sim_val_arr.size());
    for (auto i = 0; i < idx_arr_our_sol.size(); i++) { idx_arr_our_sol[i] = i; }
    std::sort(std::begin(idx_arr_our_sol), std::end(idx_arr_our_sol),
              [&sim_val_computed](int l, int r) { return sim_val_computed[l] > sim_val_computed[r]; });

    // 1st: NDCG
    double sum_ground_truth = 0;
    double sum_sol_res = 0;
    for (auto i = 0; i < k; i++) {
        sum_ground_truth += (pow(2, sim_val_arr[idx_arr[i]]) - 1) / log((double) (i + 2));
        sum_sol_res += (pow(2, sim_val_arr[idx_arr_our_sol[i]]) - 1) / log((double) (i + 2));
    }
    cout << sum_sol_res << "/" << sum_ground_truth << endl;
    cout << "NDCG:" << (sum_sol_res / sum_ground_truth) << endl;

    // 2nd: precision
    std::sort(std::begin(idx_arr), std::begin(idx_arr) + k);
    std::sort(std::begin(idx_arr_our_sol), std::begin(idx_arr_our_sol) + k);

    vector<int> intersection_arr;
    std::set_intersection(std::begin(idx_arr), std::begin(idx_arr) + k, std::begin(idx_arr_our_sol),
                          std::begin(idx_arr_our_sol) + k, back_inserter(intersection_arr));
    cout << "precision #:" << intersection_arr.size() << "/" << k << endl;
#endif
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}