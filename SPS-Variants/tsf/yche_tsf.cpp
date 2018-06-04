//
// Created by yche on 12/18/17.
//

#include <cmath>
#include <iostream>
#include <chrono>

#include "yche_tsf.h"
#include "util/pretty_print.h"

void YcheTSF::BuildIndex() {
    SFMTRand rand;
    auto tmp_start = std::chrono::high_resolution_clock::now();

    owg_arr.resize(sampleNum);
    for (int sid = 0; sid < sampleNum; ++sid) {
        owg_arr[sid] = vector<int>(n, -1);
        for (int i = 0; i < n; ++i) {
            auto &one_way_graph = owg_arr[sid];
            if (off[i] != off[i + 1]) {
                int idx = rand.uint_rand() % (off[i + 1] - off[i]);
                one_way_graph[i] = dst_v[off[i] + idx];
            }
        }
    }
    auto tmp_end = std::chrono::high_resolution_clock::now();
    cout << "indexing computation time:" << static_cast<float >(
            std::chrono::duration_cast<std::chrono::microseconds>(tmp_end - tmp_start).count() / (pow(10, 6)))
         << " s\n";
}

double YcheTSF::querySinglePair(int u, int v) {
    if (u == v) { return 1.0; }

    double sim = 0;

    // random walk on Gr for vertex u, sampleNum * sampleQueryNum times
    // use one-way graph for vertex v
    auto path_v = vector<int>(maxSteps);
    for (int sid = 0; sid < sampleNum; ++sid) {
        // init from one-way-graph for v
        auto &one_way_graph = owg_arr[sid];
        path_v[0] = v;
        for (auto step = 0; step < maxSteps; ++step) {
            if (path_v[step] >= one_way_graph.size()) {
                cout << "err" << endl;
            }
            if (path_v[step] < 0) {
                cout << "err too" << endl;
            }
            path_v[step + 1] = one_way_graph[path_v[step]];
            if (path_v[step + 1] == -1) { break; }
        }

        for (int q_sid = 0; q_sid < sampleQueryNum; ++q_sid) {
            int cur_vertex = u;
            // random walk on Gr
            for (int s = 1; s < maxSteps; ++s) {
                int nei_len = off[cur_vertex + 1] - off[cur_vertex];
                if (nei_len > 0) {
                    cur_vertex = dst_v[off[cur_vertex] + (rand.uint_rand() % nei_len)];
                    // first meet only
                    if (path_v[s] == cur_vertex) {
//                        cout << path_v[s] << cur_vertex << endl;
                        sim += pow(c, s);
                        break;
                    }
                } else {
                    break;
                }
            }
        }
    }
    return sim / (sampleQueryNum * sampleNum);
}

YcheTSF::YcheTSF(int sampleNum, int sampleQueryNum, int maxSteps, double c,
                 const vector<int> &graphSrc, const vector<int> &graphDst) : sampleNum(sampleNum),
                                                                             sampleQueryNum(sampleQueryNum),
                                                                             maxSteps(maxSteps), c(c),
                                                                             off(graphSrc), dst_v(graphDst) {
    n = static_cast<int>(graphSrc.size() - 1);
    BuildIndex();
}
