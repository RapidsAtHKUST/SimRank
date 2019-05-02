#include <iostream>
#include <chrono>
#include <set>

#include "ground_truth/graph_yche.h"
#include "ground_truth/simrank.h"
#include "ground_truth/random_pair_generator.h"

using namespace std;
using namespace std::chrono;

typedef pair<int, double> QPair;

int main(int argc, char *argv[]) {
    int truth_graph_size = 10000;
    int stepNum = 10;
    double eps = 0.01;
    double c = 0.6;
    double delta = 0.01;

    string file_path = "/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC/build/edge_list/" + string(argv[1]) + ".txt";
    // string file_path = "/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC/build/datasets/edge_list/" + string(argv[1]) + ".txt";
    string file_name = string(argv[1]);
    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
    int k = atoi(argv[4]);
    if (argc >= 6) eps = atof(argv[5]);

    GraphYche g(file_path);
    auto sample_pairs = read_sample_pairs(file_name, pair_num, round_i);
    // vector<pair<int, int>> sample_pairs;
    // sample_pairs.push_back(make_pair(0,1));
    // sample_pairs.push_back(make_pair(1,2));
    // sample_pairs.push_back(make_pair(2,3));
    // sample_pairs.push_back(make_pair(3,0));

    int n = g.n;

    /// calculate ground truth
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

    /// topk start
    auto tmp_start = std::chrono::high_resolution_clock::now();

    set<int> A, B;
    for (auto i = 0; i < pair_num; i++) {
        A.insert(sample_pairs[i].first);
        B.insert(sample_pairs[i].second);
    }
    auto as = A.size(), bs = B.size();
    
    vector<double> u[k + 1], v[2], s[as];
    v[0].resize(n); v[1].resize(n);
    for (auto i = 0; i <= k; i++) u[i].resize(n);
    for (auto i = 0; i < as; i++) s[i].resize(bs);

    bool swapped = false;
    if (A.size() < B.size()) {
        swap(A, B);
        swapped = true;
    }

    vector<double> d(n);
    for (auto i = 0; i < n; i++) {
        int od = g.out_degree(i);
        if (od == 0) {
            d[i] = 1;
        } else {
            d[i] = 1.0 * (od - 1) / od;
        }
    }
    
    // for (auto i = 0; i < n; i++) {
    //     cout << endl << i << endl << g.in_degree(i);
    //     for (auto x = g.off_in[i]; x < g.off_in[i + 1]; x++) {
    //         cout << " " << g.neighbors_in[x];
    //     }
    //     cout << endl << g.out_degree(i);
    //     for (auto x = g.off_out[i]; x < g.off_out[i + 1]; x++) {
    //         cout << " " << g.neighbors_out[x];
    //     }
    // }
    // cout << endl << as << " " << bs << endl;

    // for (auto i = 0; i <= n; i++) {
    //     cout << g.off_in[i] << " ";
    // }
    // cout << endl;
    // for (auto i = 0; i < g.off_in[n]; i++) {
    //     cout << g.neighbors_in[i] << " ";
    // }
    // cout << endl;
    // for (auto i = 0; i <= n; i++) {
    //     cout << g.off_out[i] << " ";
    // }
    // cout << endl;
    // for (auto i = 0; i < g.off_out[n]; i++) {
    //     cout << g.neighbors_out[i] << " ";
    // }
    // cout << endl;

    for (auto vb : B) {
        for (auto i = 0; i <= stepNum; i++) {
            for (auto j = 0; j < n; j++) {
                u[i][j] = 0.0;
            }
        }
        u[0][vb] = 1.0;
        for (auto l = 1; l <= stepNum; l++) {
            for (auto i = 0; i < n; i++) {
                // double uv = u[l - 1][i];
                // if (uv == 0) continue;
                // for (auto x = g.off_in[i]; x < g.off_in[i + 1]; x++) {
                for (auto x = g.off_out[i]; x < g.off_out[i + 1]; x++) {
                    int tmp = g.neighbors_out[x];
                    u[l][i] += 1.0 / g.in_degree(tmp) * u[l - 1][tmp];
                    // u[l][g.neighbors_in[x]] += uv * 1.0 / (double)g.in_degree(i);
                }
            }
        }
        for (auto i = 0; i < n; i++) {
            // v[0][i] = u[stepNum][i];
            v[0][i] = u[stepNum][i] * d[i];
            // cout << u[k][i] << " ";
        }
        // cout << endl;
        for (auto l = 1; l <= stepNum; l++) {
            // for (auto i = 0; i < n; i++) {
                // v[l & 1][i] = u[stepNum - l][i];
            // }
            for (auto i = 0; i < n; i++) {
                // if (v[1 - (l & 1)][i] == 0) continue;
                // for (auto x = g.off_out[i]; x < g.off_out[i + 1]; x++) {
                    // int tmp = g.neighbors_out[x];
                    // v[l & 1][tmp] += c * 1.0 / (double)g.in_degree(tmp) * v[1 - (l & 1)][i];
                // }
                // v[l & 1][i] = u[stepNum - l][i];
                v[l & 1][i] = u[stepNum - l][i] * d[i];
                int ind = g.in_degree(i);
                for (auto x = g.off_in[i]; x < g.off_in[i + 1]; x++) {
                    v[l & 1][i] += 1.0 *  c / ind * v[1 - (l & 1)][g.neighbors_in[x]];
                }
            }
        }
        for (auto va : A) {
            // double tmp = (va == vb ? 1 : 0);
            // int ind = g.in_degree(va);
            // for (auto x = g.off_in[va]; x < g.off_in[va + 1]; x++) {
                // tmp += 1.0 * c / ind * v[1 - (stepNum & 1)][g.neighbors_in[x]];
            // }
            // tmp *= (1.0 - c);

            double tmp = v[stepNum & 1][va];
            // if (va == vb) tmp = 0;
            // double tmp = (1.0 - c) * v[stepNum & 1][va];
            if (va == vb) tmp = 1;
            if (swapped) {
                s[vb][va] = tmp;
            } else {
                s[va][vb] = tmp;
            }
        }
    }

    vector<QPair> topk;
    for (int i = 0; i < pair_num; ++i) {
        auto u = sample_pairs[i].first;
        auto v = sample_pairs[i].second;
        double res = s[u][v];
        // cout << sample_pairs[i] << " " << res << endl;
        topk.push_back({i, res});
    }
    sort(topk.begin(), topk.end(), cmp);

    auto tmp_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = tmp_end - tmp_start;

    cout << "topk cost: " << elapsed.count() << "s\n";
    // cout << "mem size: " << getValue() << endl;

    // for (auto i = 0; i < as; i++) {
    //     for (auto j = 0; j < bs; j++) {
    //         cout << s[i][j] << "\t";
    //     }
    //     cout << endl;
    // }

    // for (auto i = 0; i < as; i++) {
    //     for (auto j = 0; j < bs; j++) {
    //         cout << ts->sim(i, j) << "\t";
    //     }
    //     cout << endl;
    // }

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

