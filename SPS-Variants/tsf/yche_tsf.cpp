//
// Created by yche on 12/18/17.
//

#include <cmath>
#include <iostream>

#include "yche_tsf.h"
#include "util/random.h"
#include "util/pretty_print.h"

void YcheTSF::BuildIndex() {
    Random rand;
    owg_arr.resize(sampleNum);
    for (int sid = 0; sid < sampleNum; ++sid) {
        owg_arr[sid] = vector<int>(n, -1);
        for (int i = 0; i < n; ++i) {
            auto &one_way_graph = owg_arr[sid];
            int r_num = rand.getRandom();
            if (graphSrc[i] != graphSrc[i + 1]) {
                int idx = r_num % (graphSrc[i + 1] - graphSrc[i]);
                one_way_graph[i] = graphDst[graphSrc[i] + idx];
            }
        }
    }
}

double YcheTSF::querySinglePair(int u, int v) {
    if (u == v) { return 1.0; }

    Random rand;
    double sim = 0;

    // random walk on Gr for vertex u, sampleNum * sampleQueryNum times
    // use one-way graph for vertex v
    auto path_v = vector<int>(maxSteps);
    for (int sid = 0; sid < sampleNum; ++sid) {
        // init from one-way-graph for v
        auto &one_way_graph = owg_arr[sid];
        path_v[0] = v;
        for (auto step = 0; step < maxSteps; ++step) {
            path_v[step + 1] = one_way_graph[path_v[step]];
            if (path_v[step + 1] == -1) { break; }
        }

        for (int q_sid = 0; q_sid < sampleQueryNum; ++q_sid) {
            int cur_vertex = u;
            // random walk on Gr
            for (int s = 1; s < maxSteps; ++s) {
                int nei_len = graphSrc[cur_vertex + 1] - graphSrc[cur_vertex];
                if (nei_len > 0) {
                    cur_vertex = graphDst[graphSrc[cur_vertex] + (rand.getRandom() % nei_len)];
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
                                                                             graphSrc(graphSrc), graphDst(graphDst) {
    n = static_cast<int>(graphSrc.size() - 1);
    BuildIndex();
}
