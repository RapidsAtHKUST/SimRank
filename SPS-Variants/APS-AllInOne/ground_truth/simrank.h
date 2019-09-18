#ifndef __SIMRANK_H__
#define __SIMRANK_H__

#include <boost/format.hpp>
#include <boost/multi_array.hpp>

#include "util/graph_yche.h"
#include "util/sparse_matrix_utils.h"
#include "extern_include.h"

using SimRank_matrix= boost::multi_array<double, 2>;

//extern void basic_simrank(GraphYche &g, double c, SimRank_matrix &);


extern void indegree_mat(const GraphYche &g, SparseMatrix<float> &P);// column normalized adjacency matrix

struct TruthSim {
    size_t n;
    MatrixXf sim;
    float epsilon;
    float c;
    string g_name;

    TruthSim() {};

    TruthSim(string name, GraphYche &g, double c_, double epsilon_);

    void run(GraphYche &g);

    void save() {
        // save to disk
        write_binary(get_file_path().c_str(), sim);
    }

    void load() {
        // load from disk
        read_binary(get_file_path().c_str(), sim);
    }

    string get_file_path() { // file path to save and load
        mkdir_if_not_exist(GROUND_TRUTH_DIR);
        return GROUND_TRUTH_DIR + str(format("GROUND_%s-%.3f.bin") % g_name % c);
    }
};

#endif
