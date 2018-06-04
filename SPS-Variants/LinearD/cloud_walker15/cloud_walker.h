#ifndef __CLOAD_WALKER_H__
#define __CLOAD_WALKER_H__

#include "../util/graph.h"

const string CLOUD_WALKER_DIR = "/homes/ywangby/workspace/LinsysSimRank/datasets/cloudwalker/";

struct CloudWalker {
    // implementation of VLDB'16 paper: Walking in the Cloud: Parallel SimRank at Scale 
    double c;
    int T;
    int L;
    int R;
    int R_prime;
    string g_name;
    size_t n; // number of nodes

    double cpu_time; // cpu time
    int mem_size; // the memory usage
    bool save_whole_sim; // control whether save the simrank matrix to disk

public:
    VectorXd D; // dynamic vector D
    DirectedG *g; // pointer to the graph

#ifdef SINGLE_SOURCE
    SparseMatrix<double> hat_P; // the column normalized version of P^{T}, column major
    VectorXd F; // denote the diag matrix $F$ of the original paper
    MatrixXd sim;
#endif

public:
    CloudWalker() = default;

    CloudWalker(DirectedG *, string name, double c_, int T_, int L_, int R_, int R_prime_);

private:
    string get_file_path_base();

    void preprocess_D();

    void build_or_load_index();

public:
#ifdef SINGLE_SOURCE
    void preprocess_F();

    void preprocess_hat_P();

    void mcss(int s, VectorXd &); // Monte Carlo Single-source query: source node, result vec
#endif

    double mcsp(int u, int v, MatrixXd &pos_dist_u, MatrixXd &pos_dist_v);

    double mcsp(int u, int v);

    void Tstep_distribution(int i, MatrixXd &pos_dist); // compute T-step position distribution of a number of samples
};

#endif
