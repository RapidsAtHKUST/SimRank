//
// Created by yche on 12/12/17.
//

#ifndef ISP_YCHE_ISP_H
#define ISP_YCHE_ISP_H

#include <sys/stat.h>

#include <string>

#include <boost/graph/adjacency_list.hpp>

#include <sparsepp/spp.h>

using namespace std;
using namespace boost;

using spp::sparse_hash_map;
using spp::sparse_hash_set;

class ISP {
private:
    sparse_hash_map<uint32_t, sparse_hash_map<uint32_t, double>> trans_mat; //read-only

private:
    vector<pair<uint32_t, uint32_t>> GetEdgeList(string &file_path);

    void DiffuseFromSinglePos(double q_k_prev_row_col, uint32_t row, uint32_t col,
                              sparse_hash_map<uint32_t, sparse_hash_map<uint32_t, double>> &q_k);

public:
    explicit ISP(string &file_path);

    double ComputeSim(uint32_t u, uint32_t v, double c, int max_k);
};

#endif //ISP_YCHE_ISP_H
