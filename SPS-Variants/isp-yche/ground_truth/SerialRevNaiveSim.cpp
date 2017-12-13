//
// Created by yche on 12/13/17.
//

#include "SerialRevNaiveSim.h"

#include <tuple>
#include <algorithm>
#include <iostream>

#include <sparsepp/spp.h>

#include "../pretty_print.h"

void SerialRevNaiveSim::InitGraph(string &file_path) {
    n = 0u;
    auto edge_lst = GetEdgeList(file_path);
#ifdef DEBUG
    cout << edge_lst << endl;
#endif
    auto out_deg_dict = spp::sparse_hash_map<uint32_t, uint32_t>();
    for (auto &edge: edge_lst) {
        uint32_t src, dst;
        std::tie(dst, src) = edge;
        // n
        n = max(n, src);
        n = max(n, dst);
        // out-deg
        if (!out_deg_dict.contains(src)) {
            out_deg_dict[src] = 0;
        }
        out_deg_dict[src] += 1;
    }
    n += 1;
    in_deg_arr.resize(n, 0);
    for (auto i = 0u; i < n; i++) {
        in_deg_arr[i] = out_deg_dict.contains(i) ? out_deg_dict[i] : 0u;
    }

    // init off, using prefix sum(exclusive)
    off.resize(n + 1);
    off[0] = 0;
    for (auto i = 0u; i < n; i++) {
        if (out_deg_dict.contains(i)) {
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
    cout << off << endl;
    cout << dst_v << endl;
    cout << in_deg_arr << endl;
#endif
}

SerialRevNaiveSim::SerialRevNaiveSim(string &file_path) {
    InitGraph(file_path);
    sim_mat_arr[0].resize(n * n, 0);
    auto &cur_sim_mat = sim_mat_arr[0];
    for (int a = 0u; a < n; a++) { cur_sim_mat[a * n + a] = 1.0; }
    sim_mat_arr[1].resize(n * n, 0);
}

vector<double> &SerialRevNaiveSim::ComputeSim(int max_iter, double eps, double c) {
    int prev_idx = 1;
    cur_idx = 0;

    bool has_change = true;
    for (auto i = 0; i < max_iter; i++) {
        if (!has_change) {
            break;
        }
        cur_idx = (cur_idx + 1) % 2;
        prev_idx = (prev_idx + 1) % 2;
        auto &cur_sim_mat = sim_mat_arr[cur_idx];
        auto &prev_sim_mat = sim_mat_arr[prev_idx];

#ifdef DEBUG
        cout << prev_sim_mat << endl;
#endif
        // clear content for the iterative computation, sim(a,a) always 1
        std::fill(begin(cur_sim_mat), end(cur_sim_mat), 0);
        for (int a = 0u; a < n; a++) { cur_sim_mat[a * n + a] = 1.0; }

        // prepare cur_sim_mat
        for (auto a = 0; a < n; a++) {
            for (auto b = 0; b < n; b++) {
                // back propagation from position (a,b)
                if (a != b) {
                    for (auto off_a = off[a]; off_a < off[a + 1]; off_a++) {
                        auto nei_a = dst_v[off_a];
                        for (auto off_b = off[b]; off_b < off[b + 1]; off_b++) {
                            auto nei_b = dst_v[off_b];
                            cur_sim_mat[a * n + b] +=
                                    c / (in_deg_arr[a] * in_deg_arr[b]) * prev_sim_mat[nei_a * n + nei_b];
                        }
                    }
                }

            }
        }
        has_change = false;
        for (auto e_i = 0u; e_i < cur_sim_mat.size(); e_i++) {
            if (cur_sim_mat[e_i] - prev_sim_mat[e_i] >= eps) {
                has_change = true;
                break;
            }
        }
#ifdef DEBUG
        cout << cur_sim_mat << endl;
#endif
    }
    return sim_mat_arr[cur_idx];
}
