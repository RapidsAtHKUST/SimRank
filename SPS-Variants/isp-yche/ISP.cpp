//
// Created by yche on 12/12/17.
//

#include "ISP.h"

#include <cmath>

#include <iostream>
#include <fstream>

#include "pretty_print.h"

vector<pair<uint32_t, uint32_t>> ISP::GetEdgeList(string &file_path) {
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
            lines.emplace_back(first, second);
        }
    }
    return lines;
}

ISP::ISP(string &file_path) {
    auto lines = GetEdgeList(file_path);
    auto count_sparse_vec = sparse_hash_map<uint32_t, uint32_t>();
    // 1st: accumulate count for each row(fixed dst_v)
    for (auto &my_pair: lines) {
        auto dst_v = my_pair.second;
        auto src_v = my_pair.first;
        if (!count_sparse_vec.contains(dst_v)) {
            count_sparse_vec.emplace(dst_v, 0);
            trans_mat[dst_v] = sparse_hash_map<uint32_t, double>();
        }
        count_sparse_vec[dst_v] += 1;
        trans_mat[dst_v][src_v] = 1;
    }
    cout << lines << endl;

    // 2nd: normalization transition matrix for each row
    uint32_t dst_v, count;
    for (auto &my_pair: count_sparse_vec) {
        std::tie(dst_v, count) = my_pair;
        auto &in_neighbors = trans_mat[dst_v];
        for (auto &in_neighbor_pair: in_neighbors) {
            in_neighbor_pair.second /= count;
        }
    }

#ifdef DEBUG
    for (auto &my_pair: trans_mat) {
        cout << my_pair.first << ":";
        for (auto &in_neighbor: my_pair.second) {
            cout << in_neighbor.second;
        }
        cout << endl;
    }
#endif
}

double ISP::ComputeSim(uint32_t u, uint32_t v, double c, int max_k) {
    if (u == v) { return 1.0; }

    auto sim_u_v = 0.0;
    auto q_k_prev = sparse_hash_map<uint32_t, sparse_hash_map<uint32_t, double>>();
    q_k_prev[u] = sparse_hash_map<uint32_t, double>();
    q_k_prev[u][v] = 1.0;
    for (auto i = 1; i <= max_k; i++) {
        auto q_k = sparse_hash_map<uint32_t, sparse_hash_map<uint32_t, double>>();

        // diffusion from q_k_prev[row][col] (where row!=col)
        for (auto &row_pair: q_k_prev) {
            auto row = row_pair.first;
            for (auto &col_pair: row_pair.second) {
                auto col = col_pair.first;
                if (row != col) {
                    DiffuseFromSinglePos(col_pair.second, row, col, q_k);
                }
            }
        }

        double m = 0;
        // accumulate first meet at depth i
        for (auto &row_pair: q_k) {
            auto row = row_pair.first;
            for (auto &col_pair: row_pair.second) {
                auto col = col_pair.first;
                if (row == col) {
                    m += col_pair.second; // i.e, q_k[row][col]
                }
            }
        }

        // add to sim score
        sim_u_v += pow(c, i) * m;
        q_k_prev = q_k;
    }
    return sim_u_v;
}

void ISP::DiffuseFromSinglePos(double q_k_prev_row_col, uint32_t row, uint32_t col,
                               sparse_hash_map<uint32_t, sparse_hash_map<uint32_t, double>> &q_k) {
    if (trans_mat.contains(row) && trans_mat.contains(col)) {
        auto &row_in_neighbor_pairs = trans_mat[row];
        auto &col_in_neighbor_pairs = trans_mat[col];

        //reserve memory for next position rows
        q_k.reserve(row_in_neighbor_pairs.size() * 2);
        for (auto &row_in_neighbor_pair: row_in_neighbor_pairs) {
            int row_in_neighbor;
            double row_in_neighbor_prob;
            std::tie(row_in_neighbor, row_in_neighbor_prob) = row_in_neighbor_pair;
            // initialize for next position row
            q_k[row_in_neighbor] = sparse_hash_map<uint32_t, double>(col_in_neighbor_pairs.size() * 2);
            auto &next_row_spare_vec = q_k[row_in_neighbor];

            for (auto &col_in_neighbor_pair: col_in_neighbor_pairs) {
                int col_in_neighbor;
                double col_in_neighbor_prob;
                std::tie(col_in_neighbor, col_in_neighbor_prob) = col_in_neighbor_pair;
                next_row_spare_vec[col_in_neighbor] =
                        q_k_prev_row_col * row_in_neighbor_prob * col_in_neighbor_prob;
            }
        }
    }
}

