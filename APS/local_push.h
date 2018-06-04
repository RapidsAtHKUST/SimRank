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

#include "graph.h"

using boost::format;
const string LOCAL_PUSH_DIR = "/export/data/ywangby_datasets/local_push/";

extern double cal_rmax(DirectedG &g, double c, double epsilon, double delta);//calculate r_,max

extern double cal_rmax(double c, double epsilon); // the r_max for general case

/* base class of local push */
struct LocalPush {
    /* data members */
    string g_name; // the name of graph data
    DensePairMap<float> P; // the estimates 
    DensePairMap<float> R; // the residuals
    // PairHashMap R; // the residuals
    queue<NodePair> Q; // the queue to hold invalid node pairs
    DensePairMap<bool> marker;
    double r_max;
//    double c;
    float c;
    size_t n;// we need to define total number of nodes in advance
    unsigned int n_push; // record number of push
    double cpu_time;// cpu_time for local push
    double epsilon; // the error bound
    int mem_size;// the memory useage

    // methods
    LocalPush() {}

    // constructor, init memebrs,
    LocalPush(DirectedG &, string, double c, double epsilon, size_t);

    void local_push(DirectedG &g); // empty funciton for local push

    // push np's residul to neighbors
    virtual void push_to_neighbors(DirectedG &g, NodePair &np, double current_residual) {}

    void save();

    void load();

    virtual string get_file_path_base() { return string(); } // get file path of local push data

    void show(); // print values

    inline void push(NodePair &pab, double inc);

    virtual void insert(DirectedG::vertex_descriptor, DirectedG::vertex_descriptor, DirectedG &g);

    virtual void remove(DirectedG::vertex_descriptor, DirectedG::vertex_descriptor, DirectedG &g);

    virtual double query_P(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b);

    virtual double query_R(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b);

    virtual void update_residual(DirectedG &g, DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) {}

    virtual double how_much_residual_to_push(DirectedG &g, NodePair &np) {}
};

/*local push using reduced system*/
struct Reduced_LocalPush : LocalPush {
    Reduced_LocalPush() {}

    Reduced_LocalPush(DirectedG &g, string name, double c_, double r_max_, size_t n_);

    // LocalPush(g, name, c_,r_max_, n_){} // invode the base constructor
    void push_to_neighbors(DirectedG &g, NodePair &np, double current_residual) override;

    /* update the residual score of R[a,b], we can assum a<b*/
    void update_residual(DirectedG &g, DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) override;

    string get_file_path_base() override; // get file path of local push data

    double how_much_residual_to_push(DirectedG &g, NodePair &np) override;

private:
    void update_residuals_by_adding_edge(DirectedG &g, DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b);

    void
    update_residuals_by_deleting_edge(DirectedG &g, DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b);

public:
    void update_edges(DirectedG &g, vector<NodePair> edges, char);

};

/* local push using full system*/
struct Full_LocalPush : LocalPush {
    Full_LocalPush() {}

    Full_LocalPush(DirectedG &g, string name, double c_, double r_max_, size_t n_);

    void push_to_neighbors(DirectedG &g, NodePair &np, double current_residual) override;

    string get_file_path_base() override; // get file path of local push data

    double how_much_residual_to_push(DirectedG &g, NodePair &np) override;

    double query_P(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) override;

    double query_R(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) override;

    void insert(DirectedG::vertex_descriptor u, DirectedG::vertex_descriptor v, DirectedG &g) override;

    void update_residual(DirectedG::vertex_descriptor u, DirectedG::vertex_descriptor v, DirectedG::vertex_descriptor a,
                         DirectedG &g, int in_deg_v);
};


// helper functions,
template<typename T>
T findMaxInterval(T arrl[], T exit[], int n) {
    // Sort arrival and exit arrays
    sort(arrl, arrl + n);
    sort(exit, exit + n);

    // guests_in indicates number of guests at a time
    int guests_in = 1, max_guests = 1;
    T time = arrl[0];
    int i = 1, j = 0;

    // Similar to merge in merge sort to process
    // all events in sorted order
    while (i < n && j < n) {
        // If next event in sorted order is arrival,
        // increment count of guests
        if (arrl[i] <= exit[j]) {
            guests_in++;

            // Update max_guests if needed
            if (guests_in > max_guests) {
                max_guests = guests_in;
                time = (exit[j] + arrl[i]) / 2;
            }
            i++;  //increment index of arrival array
        } else // If event is exit, decrement count
        {    // of guests.
            guests_in--;
            j++;
        }
    }
    // cout << "Maximum Number of interval  = " << max_guests
    //     << " at time " << time << endl;
    return time;

}

#endif

