#ifndef __LOCAL_PUSH_H__
#define __LOCAL_PUSH_H__
#include <iostream>
#include <utility>
#include <cstdio>
#include <sparsepp/spp.h>
#include <cmath>
#include <boost/graph/adjacency_list.hpp> 
#include <boost/format.hpp>
#include "../graph.h"
#include <omp.h>
#include <stack>
#include <queue>
using boost::format;
const string LOCAL_PUSH_DIR = "./datasets/local_push/";

extern double cal_rmax(DirectedG &g, double c, double epsilon, double delta);//calculate r_,max

extern double cal_rmax(double c, double epsilon); // the r_max for general case

extern bool lp_file_exists(string data_name, double c, double epsilon, size_t n, bool is_full );



/* base class of local push */
struct LocalPush{
    /* data members */
    string g_name; // the name of graph data
    DensePairMap<float> P; // the estimates 
    DensePairMap<float> R; // the residuals
    // PairHashMap R; // the residuals
    queue<NodePair> Q; // the queue to hold invalid node pairs
    DensePairMap<bool> marker;
    double r_max;
    double c;
    size_t n;// we need to define total number of nodes in advance
    unsigned int n_push; // record number of push
    double cpu_time;// cpu_time for local push
    double epsilon; // the error bound
    int mem_size;// the memory useage

    // methods
    LocalPush(){}
    // constructor, init memebrs, 
    LocalPush(DirectedG &, string , double c, double epsilon, size_t);
    void virtual local_push(DirectedG& g); // empty funciton for local push
    void virtual push_to_neighbors(DirectedG &g, NodePair &np, double current_residual){}// push np's residul to neighbors
    void save(); 
    void load();
    void init_PR(); // init or load the data of P and R
    string virtual get_file_path_base(){return string();} // get file path of local push data
    void show(); // print values
    void virtual push(NodePair& pab, double inc){}
    void insert(DirectedG::vertex_descriptor, DirectedG::vertex_descriptor, DirectedG &g);
    void remove(DirectedG::vertex_descriptor, DirectedG::vertex_descriptor, DirectedG &g);
    double virtual query_P(DirectedG::vertex_descriptor a, DirectedG:: vertex_descriptor b){}
    double virtual query_R(DirectedG::vertex_descriptor a, DirectedG:: vertex_descriptor b){}
    double query_P(NodePair & np){ return query_P(np.first, np.second); }
    double query_R(NodePair & np){return query_R(np.first, np.second);}

    double virtual how_much_residual_to_push(DirectedG &g, NodePair &np){}
    void virtual update_edges(DirectedG &g, vector<NodePair> edges, char){}
};

/* local push using reduced system */
struct Reduced_LocalPush: LocalPush {
    Reduced_LocalPush(){}
    Reduced_LocalPush(DirectedG & g, string name, double c_, double r_max_, size_t n_);
        // LocalPush(g, name, c_,r_max_, n_){} // invode the base constructor
    void push_to_neighbors(DirectedG &g, NodePair &np, double current_residual) override;


    string virtual get_file_path_base() override; // get file path of local push data

    void update_residuals_by_adding_edge(DirectedG &g, DirectedG:: vertex_descriptor a, DirectedG ::vertex_descriptor b);
    void update_edges(DirectedG &g, vector<NodePair> edges, char) override;
    void update_residuals_by_deleting_edge(DirectedG &g, DirectedG:: vertex_descriptor a, DirectedG ::vertex_descriptor b);

    double how_much_residual_to_push(DirectedG &g, NodePair &np) override;
    void push(NodePair &pab, double) override;
    double query_P(DirectedG::vertex_descriptor a, DirectedG:: vertex_descriptor b) override;
    double query_R(DirectedG::vertex_descriptor a, DirectedG:: vertex_descriptor b) override;
};

/* local push using the full linear system */
struct Full_LocalPush: LocalPush {
    Full_LocalPush(){}
    Full_LocalPush(DirectedG & g, string name, double c_, double r_max_, size_t n_);
    void push_to_neighbors(DirectedG &g, NodePair &np, double current_residual) override;
    string virtual get_file_path_base() override; // get file path of local push data
    double how_much_residual_to_push(DirectedG&g, NodePair &np) override;
    void push(NodePair &pab, double) override;
    double query_P(DirectedG::vertex_descriptor a, DirectedG:: vertex_descriptor b) override;
    double query_R(DirectedG::vertex_descriptor a, DirectedG:: vertex_descriptor b) override;
};

/* parallel local push for SimRank*/
struct Parallel_LocalPush:LocalPush{
    Parallel_LocalPush(){}
    Parallel_LocalPush(DirectedG & g, string name, double c_, double r_max_, size_t n_);
    void virtual local_push(DirectedG & g) override; // the parallel local push method
    omp_lock_t* P_lock; // locks for P
    omp_lock_t* R_lock; // locks for R
    omp_lock_t* M_lock; // locks for markers

    ~Parallel_LocalPush(){
        delete [] P_lock;
        delete [] R_lock;
        delete [] M_lock;
    }
};

#endif

