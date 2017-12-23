//
// Created by yche on 12/22/17.
//
#include <cassert>

#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>

#include "graph_yche.h"

vector<pair<int, int>> GraphYche::GetEdgeList(string &file_path) {
    vector<pair<int, int>> lines;

    ifstream ifs(file_path);

    while (ifs.good()) {
        string tmp_str;
        stringstream ss;
        std::getline(ifs, tmp_str);
        if (!ifs.good())
            break;
        if (tmp_str[0] != '#') {
            ss.clear();
            ss << tmp_str;
            int first, second;
            ss >> first >> second;
            assert(first < INT32_MAX and second < INT32_MAX);
//            assert(first != second); // no self-loop
            if (first != second)
                lines.emplace_back(first, second);
        }
    }
    return lines;
}

void GraphYche::LoadGraph(vector<pair<int, int>> &edge_lst) {
    // 1st: init n
    n = 0u;
    for (auto &edge: edge_lst) {
        int src, dst;
        std::tie(src, dst) = edge;
        n = max(n, src);
        n = max(n, dst);
    }
    n += 1;

    // 2nd: init degree array
    in_deg_arr.resize(n, 0);
    out_deg_arr.resize(n, 0);
    for (auto &edge: edge_lst) {
        int src, dst;
        std::tie(src, dst) = edge;
        out_deg_arr[src] += 1;
        in_deg_arr[dst] += 1;
    }

    // 3rd: init off, using prefix sum(exclusive)
    off_out.resize(n + 1);
    off_out[0] = 0;
    for (auto i = 0u; i < n; i++) { off_out[i + 1] = off_out[i] + out_deg_arr[i]; }

    off_in.resize(n + 1);
    off_in[0] = 0;
    for (auto i = 0u; i < n; i++) { off_in[i + 1] = off_in[i] + in_deg_arr[i]; }

    // 4th: init m
    m = off_in[n];

    // 5th: init neighbors, assume no duplicate in the edge list
    neighbors_out.resize(m);
    auto cur_off_arr = off_out; // copy for later usage
    for (auto &edge: edge_lst) {
        int src, dst;
        std::tie(src, dst) = edge;
        auto &cur_off_ref = cur_off_arr[src];
        neighbors_out[cur_off_ref] = dst;
        cur_off_ref += 1;
    }

    neighbors_in.resize(m);
    cur_off_arr = off_in;
    for (auto &edge: edge_lst) {
        int src, dst;
        std::tie(src, dst) = edge;
        auto &cur_off_ref = cur_off_arr[dst];
        neighbors_in[cur_off_ref] = src;
        cur_off_ref += 1;
    }
}

int GraphYche::in_degree(int u) {
    return in_deg_arr[u];
}

int GraphYche::out_degree(int u) {
    return out_deg_arr[u];
}

GraphYche::GraphYche(string &graph_path) {
    auto edge_lst = GetEdgeList(graph_path);
    LoadGraph(edge_lst);
}
