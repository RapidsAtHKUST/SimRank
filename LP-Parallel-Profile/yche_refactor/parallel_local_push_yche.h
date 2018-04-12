#ifndef __LOCAL_PUSH_H__
#define __LOCAL_PUSH_H__

#include <cstdio>
#include <cmath>

#include <iostream>
#include <utility>
#include <stack>
#include <queue>

#include <boost/graph/adjacency_list.hpp>
#include <boost/format.hpp>

#include <sparsepp/spp.h>

#include "../util/graph_yche.h"
#include "../util/sparse_matrix_utils.h"

using boost::format;
using PairMarker= sparse_hash_map<NodePair, bool>;

const string LOCAL_PUSH_DIR = "/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/datasets/local_push/";

inline double cal_rmax(double c, double epsilon) {
    return (1 - c) * epsilon;
} // the r_max for general case

struct FLPTask {
    int b_;
    float residual_;

    FLPTask(int b, float residual) {
        b_ = b;
        residual_ = residual;
    }
};

struct RLPTask {
    int b_;
    float residual_;
    bool is_singleton_;

    RLPTask(int b, float residual, bool is_singleton) {
        b_ = b;
        residual_ = residual;
        is_singleton_ = is_singleton;
    }
};

struct LP {
public:
    string g_name;                  // the name of graph data

    DensePairMap<float> P;          // the estimates
    DensePairMap<float> R;          // the residuals
    DensePairMap<bool> marker;

    double r_max;
    double c;
    size_t n;               // we need to define total number of nodes in advance
    double epsilon;         // the error bound
public:
    LP(GraphYche &, string, double c, double epsilon, size_t);

    virtual double query_P(int a, int b) {};

    virtual string get_file_path_base() { return string(); } // get file path of local push data

    void save();
};

/*local push using reduced system*/
struct PRLP : LP {
    vector<vector<int>> thread_local_expansion_set_lst;

    vector<vector<int>> expansion_pair_lst;

    vector<int> expansion_set_g;

    bool* is_in_expansion_set;

    vector<vector<vector<RLPTask>>> thread_local_task_hash_table_lst;

    vector<vector<RLPTask>> g_task_hash_table;

    size_t num_threads_;
public:
    PRLP(GraphYche &g, string name, double c_, double epsilon, size_t n_);

    double query_P(int a, int b) override;

    ~PRLP() {
        delete[] is_in_expansion_set;
    }

public:
    string get_file_path_base() override;

    double how_much_residual_to_push(GraphYche &g, NodePair &np);

    void push(NodePair &pab, double, bool &);

    void local_push(GraphYche &g);
};

/* local push using full system*/
struct PFLP : LP {
    string get_file_path_base() override;

    vector<vector<int>> thread_local_expansion_set_lst;

    vector<vector<int>> expansion_pair_lst;

    vector<int> expansion_set_g;

    vector<vector<FLPTask>> task_hash_table;
#ifdef HAS_OPENMP
    vector<omp_lock_t> hash_table_lock;
#endif
    size_t num_threads;
public:
    PFLP(GraphYche &g, string name, double c_, double epsilon, size_t n_);

    double query_P(int a, int b) override;

public:
    void local_push(GraphYche &g); // empty function for local push
};

#endif