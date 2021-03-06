//
// Created by yche on 1/8/18.
//

#include <chrono>

#include "../ground_truth/graph_yche.h"
#include "../ground_truth/simrank.h"

#include "cloud_walker.h"
#include "../ground_truth/random_pair_generator.h"

using namespace std;
using namespace std::chrono;

#ifdef GROUND_TRUTH
int k = 200;
#endif


int main(int argc, char *argv[]) {
    double c = 0.6;
    int L = 3;
    int R = 100;

    int T = 10;
    int R_prime = 10000;

    DirectedG g;
    auto data_name = string(argv[1]);
    string file_name = string(argv[1]);
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
#ifdef GROUND_TRUTH
    if (argc >= 5 && string(argv[4]) != string(">>") && string(argv[4]) != string(">")) { k = atoi(argv[4]); }
#endif
    load_graph(get_edge_list_path(data_name), g);
    srand(static_cast<unsigned int>(time(nullptr))); // random number generator
    std::chrono::duration<double> elapsed{};
    auto sample_pairs = read_sample_pairs(file_name, pair_num, round_i);

    // 1st: indexing
    auto start = std::chrono::high_resolution_clock::now();
    CloudWalker cw(&g, data_name, c, T, L, R, R_prime);
    auto pre_time = std::chrono::high_resolution_clock::now();
    cout << "constructing time:" << float(duration_cast<microseconds>(pre_time - start).count()) / (pow(10, 6))
         << " s\n";

    // 2nd: query
#ifdef GROUND_TRUTH
    string path = get_edge_list_path(data_name);
    GraphYche g_gt(path);
    TruthSim ts(string(argv[1]), g_gt, c, 0.01);
    auto max_err = 0.0;
    auto failure_count = 0;

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

    start = std::chrono::high_resolution_clock::now();
    auto clock_start = clock();
#pragma omp parallel
    {
        // allocate memory in advance
        MatrixXd pos_dist_v(T + 1, cw.n);
        MatrixXd pos_dist_u(T + 1, cw.n);
#ifdef GROUND_TRUTH
#pragma omp for reduction(max:max_err) schedule(dynamic, 100)
#else
#pragma omp for schedule(dynamic, 1)
#endif
        for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
            auto u = sample_pairs[pair_i].first;
            auto v = sample_pairs[pair_i].second;
#ifdef GROUND_TRUTH
            auto res = cw.mcsp(u, v, pos_dist_u, pos_dist_v);
            sim_val_computed[pair_i] = static_cast<float>(res);

            max_err = max(max_err, abs(ts.sim(u, v) - res));
            if (abs(ts.sim(u, v) - res) > 0.01) {
#pragma omp critical
                {
                    cout << u << ", " << v << "," << ts.sim(u, v) << "," << res << endl;
                    failure_count++;
                }
            }
#else
            cw.mcsp(u, v, pos_dist_u, pos_dist_v);
#endif
        }
    };

    auto end = std::chrono::high_resolution_clock::now();
    auto clock_end = clock();
    cout << "total query cpu time:" << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s" << endl;
    elapsed = end - start;

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

    // 3rd: avg difference
    double difference_accumulation = 0;
    for (auto idx: intersection_arr) {
        difference_accumulation += abs(sim_val_computed[idx] - sim_val_arr[idx]);
    }
    cout << "avg difference:" << (difference_accumulation / intersection_arr.size()) << endl;

#endif
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}