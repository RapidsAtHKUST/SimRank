#ifndef __SIMRANK_H__
#define __SIMRANK_H__

#include <boost/format.hpp>
#include <boost/multi_array.hpp>
#include "util/util.h"

#include "graph_yche.h"
#include "sparse_matrix_utils.h"

using SimRank_matrix = boost::multi_array<double, 2>;

//extern void basic_simrank(GraphYche &g, double c, SimRank_matrix &);

const string GROUND_TRUTH_DIR(
        string(DATA_ROOT) + "/ground_truth/");

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
        exec(string("mkdir -p " + GROUND_TRUTH_DIR).c_str());
        string file_path = GROUND_TRUTH_DIR + str(format("GROUND_%s-%.3f.bin") % g_name % c);
        return file_path;
    }
};

#endif
