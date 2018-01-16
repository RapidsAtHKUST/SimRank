#ifndef __LINEAR_D_H__
#define __LINEAR_D_H__

#include <memory>

#include "../util/graph.h"

const string LINEAR_D_DIR = "/homes/ywangby/workspace/LinsysSimRank/datasets/linearD/";

struct LinearD {
    // The Linearization Technique SimRank (Efficient SimRank Computation via Linearization KDD'14)
public:
    string g_name;
    size_t n; // number of nodes

    double cpu_time; // cpu time
    int mem_size; // the memory usage

public:
    double c;
    int T;
    int L;
    int R;

    VectorXd D; // dynamic vector D
    DirectedG *g; // pointer to the graph
    MatrixXd sim;
    SparseMatrix<double> P, PT;

private:
    string get_file_path_base();

    string get_p_file_path_base();

    void build_or_load_index();

public:
    LinearD() = default;

    LinearD(DirectedG *, string name, double c_, int T_, int L_, int R_);

    LinearD(string name, double c_, int T_, int L_, int R_);

#ifdef SAVE_LOAD
    void save();

    void load();
#endif

    pair<double, double> estimate_SDkk_SEkk(int k); // estimate S^{L}(D)_{kk} and S^{L}(E^{kk})_{kk}

    void compute_D();

    void compute_P();

public:
    void single_source(int i, VectorXd &r);

    double single_pair(int i, int j);

    double single_pair(int i, int j, VectorXd &lhs_vec, VectorXd &rhs_vec);

    void all_pair();
};

#endif
