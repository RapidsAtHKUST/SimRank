#include <iostream>
#include <chrono>
#include <set>

#include "yche_tsf.h"
#include "ground_truth/graph_yche.h"
#include "ground_truth/simrank.h"
#include "ground_truth/stat.h"
#include "input_output.h"
#include "ground_truth/random_pair_generator.h"

using namespace std;
using namespace std::chrono;

typedef pair<int, double> QPair;
double eps = 0.01;

int main(int argc, char *argv[]) {
    auto decayFactor = 0.6;
    auto stepNum = 11;
    auto sampleNum = 100;
    int truth_graph_size = 10000;

    string file_path = "/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC/build/edge_list/" + string(argv[1]) + ".txt";
    string file_name = string(argv[1]);
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
    int k = atoi(argv[4]);
    if (argc >= 6) eps = atof(argv[5]);

    double c = 0.6;
    double delta = 0.01;
    double b = min(1.0, c / (1 - c));
    auto sampleQueryNum = static_cast<int>(ceil(log(delta / 2) / (-2) / (pow(eps, 2)) * (pow(b - c, 2)) / sampleNum));
    auto sample_pairs = read_sample_pairs(file_name, pair_num, round_i);

    vector<int> graph_src_vec;
    vector<int> graph_dst_vec;

    int n = (int)InitGraph(file_path, graph_src_vec, graph_dst_vec);

    auto tmp_start = std::chrono::high_resolution_clock::now();
    auto yche_tfs = YcheTSF(sampleNum, sampleQueryNum, stepNum, decayFactor, graph_src_vec, graph_dst_vec);
    auto tmp_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = tmp_end - tmp_start;

    cout << "indexing time: " << elapsed.count() << "s\n";
    cout << "mem size: " << getValue() << endl;
    
    TruthSim *ts;
    vector<QPair> ts_topk;
    set<NodePair> ts_set;
    double zk = 0.0;
    auto cmp = [](QPair x, QPair y) {return x.second > y.second;};

    if (n < truth_graph_size) {
        GraphYche g_gt(file_path);
        ts = new TruthSim(file_name, g_gt, c, 0.01);
        for (int i = 0; i < pair_num; ++i) {
            auto u = sample_pairs[i].first;
            auto v = sample_pairs[i].second;
            ts_topk.push_back({i, ts->sim(u, v)});
        }
        sort(ts_topk.begin(), ts_topk.end(), cmp);
        for (int i = 0; i < k; ++i) {
            ts_set.insert(sample_pairs[ts_topk[i].first]);
            zk += (pow(2, ts_topk[i].second) - 1) / log2(i + 2);
        }
    }

    tmp_start = std::chrono::high_resolution_clock::now();
    vector<QPair> topk;
    for (int i = 0; i < pair_num; ++i) {
        auto u = sample_pairs[i].first;
        auto v = sample_pairs[i].second;
        double res = yche_tfs.querySinglePair(u, v);
        // cout << sample_pairs[i] << " " << res << endl;
        topk.push_back({i, res});
    }
    sort(topk.begin(), topk.end(), cmp);
    tmp_end = std::chrono::high_resolution_clock::now();
    elapsed = tmp_end - tmp_start;
    cout << "topk cost: " << elapsed.count() << endl;

    double max_err = 0.0, ndcg = 0.0;
    int intersec = 0;
    if (n < truth_graph_size) {
        for (int i = 0; i < k; ++i) {
            topk[i].second = ts->sim(sample_pairs[topk[i].first].first, sample_pairs[topk[i].first].second);
        }
        sort(topk.begin(), topk.begin() + k, cmp);
        for (int i = 0; i < k; ++i) {
            double abs_error = abs(ts_topk[i].second - topk[i].second);
            max_err = max(max_err, abs_error);
            if (ts_set.find(sample_pairs[topk[i].first]) != ts_set.end()) ++intersec;
            ndcg += (pow(2, topk[i].second) - 1) / log2(i + 2);
        }
        cout << "max err: " << max_err << ", precison: " << ((double)intersec / k) << ", ndcg: " << (ndcg / zk) << endl;
    }

    return 0;
}
