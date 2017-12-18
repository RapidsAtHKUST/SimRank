//
// Created by yche on 12/13/17.
//
#include "input_output.h"

#include <cassert>

#include <fstream>
#include <sstream>
#include <iostream>
#include <tuple>
#include <unordered_map>

using namespace std;

vector<pair<uint32_t, uint32_t>> GetEdgeList(string &file_path) {
    vector<pair<uint32_t, uint32_t>> lines;

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

void InitGraph(string &file_path, vector<int> &off, vector<int> &dst_v) {
    auto n = 0u;
    auto edge_lst = GetEdgeList(file_path);

    auto out_deg_dict = unordered_map<uint32_t, uint32_t>();
    for (auto &edge: edge_lst) {
        uint32_t src, dst;
        std::tie(dst, src) = edge;
        // n
        n = max(n, src);
        n = max(n, dst);
        // out-deg
        if (out_deg_dict.find(src) == out_deg_dict.end()) {
            out_deg_dict[src] = 0;
        }
        out_deg_dict[src] += 1;
    }
    n += 1;

    // init off, using prefix sum(exclusive)
    off.resize(n + 1);
    off[0] = 0;
    for (auto i = 0u; i < n; i++) {
        if (out_deg_dict.find(i) != out_deg_dict.end()) {
            off[i + 1] = off[i] + out_deg_dict[i];
        } else {
            off[i + 1] = off[i];
        }
    }

    // init dst_v, assume no duplicate in the edge list
    dst_v.resize(off[n]);
    auto cur_off = off; // copy
    for (auto &edge: edge_lst) {
        uint32_t src, dst;
        std::tie(dst, src) = edge;
        auto &cur_off_ref = cur_off[src];
        dst_v[cur_off_ref] = dst;
        cur_off_ref += 1;
    }

#ifdef DEBUG
    cout << "total vertex number:" << n << endl;
#endif
}