//
// Created by yche on 12/23/17.
//

#include <ctime>

#include "../../yche_refactor/flpmc_yche.h"
#include "../../util/graph_yche.h"
#include "../../yche_refactor/simrank.h"
#include "../../util/random_pair_generator.h"
#include "../../playground/pretty_print.h"

string get_new_graph_path(string data_name){
    return string("/csproject/biggraph/ywangby/yche/git-repos/SimRank/LPMC/build/edge_list/") + data_name + string(".txt");
}

typedef pair<int, double> QPair;

int main(int argc, char *argv[]) {
    string data_name(argv[1]);
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
    int k = atoi(argv[4]);

    double c = 0.6;
    double eps = 0.01;
    if (argc >= 6) eps = atof(argv[5]);
    double delta = 0.01;
    if (argc >= 7) delta = atof(argv[6]);

    string path = get_new_graph_path(data_name);
    auto start = std::chrono::high_resolution_clock::now();
    GraphYche g(data_name);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cout << "graph load time: " << elapsed.count() << endl;
    int truth_graph_size = 10000;

    size_t n = g.n;

    start = std::chrono::high_resolution_clock::now();
#ifdef VARYING_RMAX
    auto flpmc = FLPMC(data_name, g, c, eps, delta, 100, 0.14);
#else
    auto flpmc = FLPMC(data_name, g, c, eps, delta, 100);
#endif
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << "indexing time: " << elapsed.count() << endl;
    cout << "mem size: " << getValue() << endl;

    auto queries = read_sample_pairs(data_name, pair_num, round_i); // change path

    TruthSim *ts;
    vector<QPair> ts_topk;
    set<NodePair> ts_set;
    double zk = 0.0;
    auto cmp = [](QPair x, QPair y) {return x.second > y.second;};

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
    }

    double max_err = 0.0;
    start = std::chrono::high_resolution_clock::now();
    vector<QPair> topk;
    for (int i = 0; i < queries.size(); ++i) {
        double r = flpmc.query_one2one(queries[i]);
        topk.push_back({i, r});
    }
    sort(topk.begin(), topk.end(), cmp);
    topk.resize(k);
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << "topk cost: " << elapsed.count() << endl;

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
}
