//
// Created by yche on 12/17/17.
//

#include <cmath>

#include <iostream>
#include <chrono>

#include "../algorithm/readsrq.h"
#include "../util/sample_num.h"
#include "../algorithm/readsd.h"

#include "../ground_truth/graph_yche.h"
#include "../ground_truth/simrank.h"
#include "../ground_truth/random_pair_generator.h"

using namespace std;
using namespace std::chrono;

int k = 200;
double eps = 0.01;

// usage and example:
// g++ reads*.h reads*.cpp timer.h -O3 -w -std=c++11 test.cpp -I . && ./a.out hp.data 3133
int main(int argc, char **argv) {
    // 1st: eps, delta and c for computing sample number
//    double eps = 0.008;
    double delta = 0.01;
    double c = 0.6;
    int r = 100;
    random_device rd;
    srand(rd());
    cout << "argc:" << argc << endl;
    bool is_varying_param = false;
    if (argc >= 7 && string(argv[5]) != string(">") && string(argv[5]) != string(">>")) {
        cout << "varying parameters" << endl;
        c = atof(argv[4]);
        eps = atof(argv[5]);
        delta = atof(argv[6]);
        cout << boost::format("c:%s, eps:%s, delta:%s") % c % eps % delta << endl;
        is_varying_param = true;
    }

    // 2nd: t(max length), n
    int t = 10;
    auto data_name = string(argv[1]);
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
    if (!is_varying_param) {
        if (argc >= 5 && string(argv[4]) != string(">>") && string(argv[4]) != string(">")) {
            k = atoi(argv[4]);
            eps = atof(argv[5]);
        }
    }

    int rq = compute_reads_rq_num(eps, delta, c, r);
    cout << "sample num:" << r << ", on-line rand-walk:" << rq << endl;
    auto full_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + data_name + ".txt";
    GraphYche g_gt(full_path);
    int n = g_gt.n;
    cout << "vertex num:" << n << endl;

    // 3rd: construct index
    auto start = high_resolution_clock::now();
    readsrq i3(data_name, n, r, rq, c, t);
    auto end = high_resolution_clock::now();
    cout << "construct time:" << duration_cast<microseconds>(end - start).count() / pow(10, 6) << " s\n";

    auto sample_pairs = read_sample_pairs(data_name, pair_num, round_i);
    // 4th: query
    if (n < 10000) {
        double max_err = 0;
        TruthSim ts(data_name, g_gt, c, 0.01);

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

        auto tmp_start = high_resolution_clock::now();
        auto clock_start = clock();

#pragma omp parallel
        {
            auto *ansVal = new double[n];

#pragma omp for reduction(max:max_err) schedule(dynamic, 100)
            for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
                auto u = sample_pairs[pair_i].first;
                auto v = sample_pairs[pair_i].second;

                auto result = i3.queryOne(u, v, ansVal);
                sim_val_computed[pair_i] = static_cast<float>(result);

                max_err = max(max_err, abs(ts.sim(u, v) - result));
                if (abs(ts.sim(u, v) - result) > 0.01) {
#pragma omp critical
                    cout << u << ", " << v << "," << ts.sim(u, v) << "," << result << endl;
                }
            }
            delete[] ansVal;
        };
        auto tmp_end = high_resolution_clock::now();
        auto clock_end = clock();
        cout << "total query cpu time:" << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s" << endl;
        cout << "query time:" << duration_cast<microseconds>(tmp_end - tmp_start).count() / pow(10, 6) << " s\n";
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

        // 3rd: avg difference
        double difference_accumulation = 0;
        for (auto idx: intersection_arr) {
            difference_accumulation += abs(sim_val_computed[idx] - sim_val_arr[idx]);
        }
        cout << "avg difference:" << (difference_accumulation / intersection_arr.size()) << endl;

    } else {
        auto tmp_start = high_resolution_clock::now();
        auto clock_start = clock();

#pragma omp parallel
        {
            auto *ansVal = new double[n];

#pragma omp for schedule(dynamic, 100)
            for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
                auto u = sample_pairs[pair_i].first;
                auto v = sample_pairs[pair_i].second;
                auto result = i3.queryOne(u, v, ansVal);
            }
            delete[] ansVal;
        };
        auto tmp_end = high_resolution_clock::now();
        auto clock_end = clock();
        cout << "total query cpu time:" << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s"
             << endl;
        cout << "query time:" << duration_cast<microseconds>(tmp_end - tmp_start).count() / pow(10, 6) << " s\n";
    }
}