//
// Created by yche on 12/13/17.
//

#ifndef ISP_YCHE_SERIALREVNAIVESIM_H
#define ISP_YCHE_SERIALREVNAIVESIM_H

#include <array>

#include "../input_output.h"

class SerialRevNaiveSim {
private:
    // csr graph for reversed graph
    uint32_t n; // total vertex # [0, n)

    vector<uint32_t> off;
    vector<uint32_t> dst_v;

    // vertex property
    vector<uint32_t> in_deg_arr;    // for original graph

    // sim matrices
    int cur_idx;
    array<vector<double>, 2> sim_mat_arr;
private:
    void InitGraph(string &file_path);

public:
    explicit SerialRevNaiveSim(string &file_path);

    vector<double>& ComputeSim(int max_iter, double eps, double c);
};


#endif //ISP_YCHE_SERIALREVNAIVESIM_H
