//
// Created by yche on 12/22/17.
//
#include <cassert>

#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>
#include <chrono>

using namespace std::chrono;

#include "graph_yche.h"

vector<pair<int, int>> GetEdgeList(string &file_path) {
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
//            if (first != second)
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
    auto start_time = high_resolution_clock::now();
    auto edge_lst = vector<pair<int, int>>();

    if (file_exists(get_bin_list_path_from_txt(graph_path))) {
        cout << "load from binary file..." << endl;
        ReadFileToArr(get_bin_list_path_from_txt(graph_path), edge_lst);
    } else {
        edge_lst = GetEdgeList(graph_path);
    }

    auto end_time = high_resolution_clock::now();
    cout << "load edge list time:" << duration_cast<milliseconds>(end_time - start_time).count()
         << " ms\n";

    LoadGraph(edge_lst);
    auto final_time = high_resolution_clock::now();
    cout << "parse edge list to bi-dir csr time:" << duration_cast<milliseconds>(final_time - end_time).count()
         << " ms\n";
}

bool GraphYche::BinarySearch(uint32_t offset_beg, uint32_t offset_end, int val) {
    if (offset_beg >= offset_end) { return false; }

    auto mid = static_cast<uint32_t>((static_cast<unsigned long>(offset_beg) + offset_end) / 2);
    if (neighbors_out[mid] == val) { return true; }

    return val < neighbors_out[mid] ? BinarySearch(offset_beg, mid, val) :
           BinarySearch(mid + 1, offset_end, val);
}

bool GraphYche::exists_edge(int src, int dst) {
//    for (auto offset = off_out[src]; offset < off_out[src + 1]; offset++) {
//        if (neighbors_out[offset] == dst) {
//            return true;
//        }
//    }
//    return false;
    return BinarySearch(static_cast<uint32_t>(off_out[src]), static_cast<uint32_t>(off_out[src + 1]), dst);
}

#if !defined(SFMT)
int sample_in_neighbor(int a, GraphYche &g) {
    // sample one in-neighbor of node a in g
    auto in_deg = g.in_deg_arr[a];
    if (in_deg > 0) {
        return g.neighbors_in[select_randomly(g.off_in[a], g.off_in[a + 1])];
    } else {
        return -1;
    }
}
#else

//int sample_in_neighbor(int a, GraphYche &g, SFMTRand &sfmt_rand_gen) {
//    auto in_deg = g.in_deg_arr[a];
//    if (in_deg > 0) {
//        return g.neighbors_in[select_randomly_sfmt(g.off_in[a], g.off_in[a + 1], sfmt_rand_gen)];
//    } else {
//        return -1;
//    }
//}

#endif