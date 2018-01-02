#ifndef __CLOAD_WALKER_H__
#define __CLOAD_WALKER_H__

#include "../util/graph.h"

const string CLOUD_WALKER_DIR = "./datasets/cloudwalker/";

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
    VectorXd D; // dynamic vector D
    DirectedG *g; // pointer to the graph
    SparseMatrix<double> hat_P; // the column normalized version of P^{T}, column major
    VectorXd F; // denote the diag matrix $F$ of the original paper
    MatrixXd sim;

    CloudWalker() {}

    CloudWalker(DirectedG *, string name, double c_, int T_, int L_, int R_, int R_prime_);

    void preprocess_D();

    void preprocess_F();

    void preprocess_hat_P();

    void mcss(int s, VectorXd &); // Monte Carlo Single-source query: source node, result vec
    void mcap(); // Monte Carlo All-pair SimRank computation  
    void Tstep_distribution(int i, int samples,
                            MatrixXd &pos_dist); // compute T-step position distribution of a number of samples
    string get_file_path_base();

    void save();

    void load();

};

#endif
