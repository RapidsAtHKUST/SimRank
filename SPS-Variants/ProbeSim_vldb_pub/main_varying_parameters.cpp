//
// Created by yche on 7/24/18.
//

#include <chrono>

#include <ground_truth/random_pair_generator.h>

#ifdef GROUND_TRUTH
#include <ground_truth/simrank.h>
#endif

#include "Graph.h"
#include "SimStruct.h"

using namespace boost;

#ifdef GROUND_TRUTH
int k = 200;
#endif

int main(int argc, char *argv[]) {
    // 1st: load graph
    string file_name = argv[1];
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);

    double c = atof(argv[4]);
    double eps = atof(argv[5]);
    double delta = atof(argv[6]);

#ifdef GROUND_TRUTH
    if(argc > 7){
        k = atoi(argv[7]);
    }
#endif

    string file_path = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + file_name + ".txt";
    auto sample_pairs = read_sample_pairs(file_name, pair_num, round_i);

    // 2nd: init gt, sample pairs
#ifdef GROUND_TRUTH
    cout << "k: " << k << endl;
    GraphYche g_gt(file_path);
    TruthSim ts(file_name, g_gt, c, 0.01);
    auto failure_count = 0;
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
    auto start = std::chrono::high_resolution_clock::now();
    auto clock_start = clock();

    SimStruct simStruct = SimStruct(file_path, c, eps, delta);
    // 3rd: querying pairs

#pragma omp parallel
    {
        auto probe_sim = SimStruct(simStruct, eps);
        auto *resultList = new double[probe_sim.g.n];
#ifdef GROUND_TRUTH
#pragma omp for reduction(max:max_err) schedule(dynamic, 100)
#else
#pragma omp for schedule(dynamic, 100)
#endif
        for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
            auto u = sample_pairs[pair_i].first;
            auto v = sample_pairs[pair_i].second;
#ifdef GROUND_TRUTH
            auto res = probe_sim.batch_for_single_pair(u, v, resultList);
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
            probe_sim.batch_for_single_pair(u, v, resultList);
#endif
        }
        delete[]resultList;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto clock_end = clock();

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
    cout << "total query cpu time:" << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s" << endl;
    std::chrono::duration<double> elapsed = end - start;
#ifdef GROUND_TRUTH
    cout << "max err:" << max_err << endl;
#endif
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}
