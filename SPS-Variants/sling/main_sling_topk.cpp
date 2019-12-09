#include <iostream>

#include "ground_truth/simrank.h"

#include "sling.h"
#include "ground_truth/stat.h"
#include "ground_truth/random_pair_generator.h"

using namespace std;
using namespace std::chrono;

typedef pair<int, double> QPair;

int main(int argc, char *argv[]) {
    // init parameters
    Graph g;
    string file_name = argv[1];
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
    int k = atoi(argv[4]);
    int truth_graph_size = 10000;

    double c = 0.6; // atof(argv[4]);
    double eps = 0.01; 
    if (argc >= 6)
        eps = atof(argv[5]);
    double delta = 0.01; // atof(argv[6]);
    if (argc >= 7) delta = atof(argv[6]);

    double half_eps = eps / 2;
    double eps_d = (1 - c) * half_eps;
    double denominator = (2 * sqrt(c)) / (1 - sqrt(c)) / (1 - c);
    double theta = half_eps / (denominator);
    cout << "eps_d: " << eps_d << ", theta: " << theta << endl;

    // input graph
    string file_path = "/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC/build/edge_list/" + file_name + ".txt";
    g.inputGraph(file_path);
    int n = g.n;

    // init sling
    auto tmp_start = std::chrono::high_resolution_clock::now();
    Sling sling_algo(&g, file_name, c, eps_d, theta);
    sling_algo.failure_probability = delta;
    cout << "delta (fail-prob): " << sling_algo.failure_probability << endl;
    auto tmp_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = tmp_end - tmp_start;
    auto idx_time = elapsed.count();
    cout << "finish input graph: " << elapsed.count() << "s" << endl;

    tmp_start = std::chrono::high_resolution_clock::now();
    // auto clock_start_cal_d = clock();
    sling_algo.calcD(eps_d);
    // auto clock_end_cal_d = clock();
    tmp_end = std::chrono::high_resolution_clock::now();
    elapsed = tmp_end - tmp_start;
    idx_time += elapsed.count();
    // cout << "total calD cpu time: " << static_cast<double>(clock_end_cal_d - clock_start_cal_d) / CLOCKS_PER_SEC << "s" << endl;
    cout << "finish calcD: " << elapsed.count() << "s" << endl;

    tmp_start = std::chrono::high_resolution_clock::now();
    sling_algo.backward(theta);
    tmp_end = std::chrono::high_resolution_clock::now();
    elapsed = tmp_end - tmp_start;
    idx_time += elapsed.count();
    cout << "finish backward: " << elapsed.count() << "s" << endl;
    cout << "indexing time: " << idx_time << endl;
    cout << "mem size: " << getValue() << endl;

    // init ground truth
    auto sample_pairs = read_sample_pairs(file_name, pair_num, round_i);

    TruthSim *ts;
    vector<QPair> ts_topk;
    set<NodePair> ts_set;
    double zk = 0.0;
    auto cmp = [](QPair x, QPair y) {return x.second > y.second;};
    
    if (n < truth_graph_size) {
        GraphYche tsg(file_path);
        ts = new TruthSim(file_name, tsg, c, 0.01);
        for (int pair_i = 0; pair_i < pair_num; ++pair_i) {
            auto u = sample_pairs[pair_i].first;
            auto v = sample_pairs[pair_i].second;
            ts_topk.push_back({pair_i, ts->sim(u, v)});
        }
    
        sort(ts_topk.begin(), ts_topk.end(), cmp);

        for (int i = 0; i < k; ++i) {
            // cout << sample_pairs[ts_topk[i].first] << " " << ts_topk[i].second << endl;
            ts_set.insert(sample_pairs[ts_topk[i].first]);
            zk += (pow(2, ts_topk[i].second) - 1) / log2(i + 2);
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    // auto clock_start = clock();

    vector<QPair> topk;
    for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
        auto u = sample_pairs[pair_i].first;
        auto v = sample_pairs[pair_i].second;
        double res = sling_algo.simrank(u, v);
        topk.push_back({pair_i, res});
    }

    sort(topk.begin(), topk.end(), cmp);
    auto end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << format("topk cost: %s") % elapsed.count() << endl;

    double max_err = 0.0, ndcg = 0.0;
    int intersec = 0;
    if (n < truth_graph_size) {
        for (int i = 0; i < k; ++i) {
            topk[i].second = ts->sim(sample_pairs[topk[i].first].first, sample_pairs[topk[i].first].second);
        }
        sort(topk.begin(), topk.begin() + k, cmp);
        for (auto i = 0; i < k; ++i) {
            // cout << sample_pairs[topk[i].first] << " " << topk[i].second << endl;
            double abs_error = abs(ts_topk[i].second - topk[i].second);
            max_err = max(max_err, abs_error);
            if (ts_set.find(sample_pairs[topk[i].first]) != ts_set.end()) ++intersec;
            ndcg += (pow(2, topk[i].second) - 1) / log2(i + 2);
        }
        cout << "max err: " << max_err << ", precison: " << ((double)intersec / k) << ", ndcg: " << (ndcg / zk) << endl;
    }

    // auto clock_end = clock();
    // cout << "total query cpu time: " << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s" << endl;

    return 0;
}
