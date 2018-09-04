#include <cstdlib>
#include <ctime>

#include <iostream>
#include <fstream>
#include <unordered_set>

#include <boost/program_options.hpp>

#include "stat.h"
#include "bprw.h"
#include "simrank.h"
#include <util/graph_yche.h>
#include <util/random_pair_generator.h>
#include "rw_hub.h"
#include "fgi.h"
#include "phf.h"
#include "minimal_perfect_hash.h"

using namespace std;
using namespace std::chrono;
using namespace boost::program_options;
using namespace bf;

string get_new_graph_path(string data_name){
    return string("/csproject/biggraph/ywangby/yche/git-repos/SimRank/LPMC/build/edge_list/") + data_name + string(".txt");
}

int main(int argc, char *argv[]) {
    string data_name = argv[1];
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
    int k = atoi(argv[4]);
    
    double c = 0.6;
    double eps = 0.01;
    if (argc >= 6) eps = atof(argv[5]);
    double delta = 0.01;

    string path = get_new_graph_path(data_name);
    auto start = std::chrono::high_resolution_clock::now();
    // GraphYche g(path);
    GraphYche g(data_name);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cout << "graph load time: " << elapsed.count() << endl;
    int truth_graph_size = 10000;

    BLPMC_Config config;
    size_t n = g.n;
    config.is_use_linear_regression_cost_estimation = true;
    config.is_use_hub_idx = false;
    config.is_use_fg_idx = false;
    
    start = std::chrono::high_resolution_clock::now();
    BackPush bprw(data_name, g, c, eps, delta, config);
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << "indexing time: " << elapsed.count() << endl;
    cout << "mem size: " << getValue() << endl;

    // vector<NodePair> queries = read_sample_pairs(data_name, pair_num, round_i);
    
    TruthSim *ts;
    vector<QPair> ts_topk;
    set<NodePair> ts_set;
    double zk = 0.0;
    auto cmp = [](QPair x, QPair y) {return x.second > y.second;};
    /*
    if (n < truth_graph_size) {
        ts = new TruthSim(data_name, g, c, eps);
        for (int i = 0; i < pair_num; ++i) {
            ts_topk.push_back({i, ts->sim(queries[i].first, queries[i].second)});
        }
        sort(ts_topk.begin(), ts_topk.end(), cmp);
        for (int i = 0; i < k; ++i) {
            ts_set.insert(queries[ts_topk[i].first]);
            zk += (pow(2, ts_topk[i].second) - 1) / log2(i + 2);
        }
    }*/
/*
    double max_err = 0.0;
    start = std::chrono::high_resolution_clock::now();
    auto topk = bprw.top_k_sort(queries, k);
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << "topk cost: " << elapsed.count() << endl;

    for (int i = 0; i < k; ++i) {
        cout << topk[i].second << endl;
    }
*/
    int chosen = 0;
    for (int i = n - 1; i >= 0; --i) {
        if (i % 10000 == 0) cout << i << endl;
        for (int j = i - 1; j >= i - 1000 && j >= 0; --j) {
            double v = bprw.query_one2one({i, j});
            if (v > eps) {
                cout << "(" << i << ", " << j << ")" << endl;
                ++chosen;
                break;
            }
        }
        if (chosen == 100) break;
    }
    /*
    if (n < truth_graph_size) {
        double ndcg = 0.0;
        int exceed = 0, intersec = 0;
        for (int i = 0; i < k; ++i) {
            topk[i].second = ts->sim(queries[topk[i].first].first, queries[topk[i].first].second);
        }
        sort(topk.begin(), topk.begin() + k, cmp);
        for (int i = 0; i < k; ++i) {
            double abs_error = abs(ts_topk[i].second - topk[i].second);
            max_err = max(max_err, abs_error);
            if (abs_error > eps) ++exceed;
            if (ts_set.find(queries[topk[i].first]) != ts_set.end()) ++intersec;
            ndcg += (pow(2, topk[i].second) - 1) / log2(i + 2);
        }
        cout << "max err: " << max_err << ", precison: " << ((double)intersec / k) << ", ndcg: " << (ndcg / zk) << endl;
    }
*/
    return 0;
}

