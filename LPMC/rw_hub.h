#ifndef __RW_HUB_H__
#define __RW_HUB_H__


#include "stat.h"
#include <boost/dynamic_bitset.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/format.hpp> 

#include <util/graph_yche.h>
#include <util/sfmt_based_rand.h>
#include <util/sparse_matrix_utils.h> // the sparse hash map definition

#include "phf.h"



using namespace boost;
using namespace boost::heap;

typedef GraphYche DirectedG;

struct greater
{
    template<class T>
    bool operator()(T const &a, T const &b) const { return a > b; }
};

struct sort_hub_pred { // sort the utility in decreasing order
    bool operator()(const std::pair<size_t,double> &left, const std::pair<size_t,double> &right) {
        return left.second > right.second;
    }
};


struct Hub_Item{
    NodePair np; // the hub of the graph node pairs
    double utility;
    NodePair position; // the cursor in the sorted utility array
    Hub_Item(NodePair & np_, double utility_, NodePair & np2):np(np_), utility(utility_), position(np2){
    }
    Hub_Item(const Hub_Item & other){
        np = other.np;
        utility = other.utility;
        position = other.position;
    }
	bool operator<(const Hub_Item& rhs) const{
        return utility < rhs.utility;
    }

};

inline Hub_Item utility_hub_converter(int a, int b, vector<pair<size_t, double>>& utility){
    // produce the hub item based on the position a, b of utility array
    NodePair np{utility[a].first, utility[b].first};
    double u = utility[a].second * utility[b].second;
    NodePair position{a,b};
    return Hub_Item(np, u, position);
}

struct Rw_Hubs{ 
    // the container for hubs
    typedef sparse_hash_map<NodePair, vector<int>> Prefix_Sum; // the prefix sum of 1s of of this node pair, size: N * K
    typedef sparse_hash_set<size_t> Single_Set; // just a set of integers for the right part of a hub
    typedef sparse_hash_map<size_t, pair<vector<bool>, size_t>> Hub_Bit_Map; // key: the right part of a hub, value: bitmap, current iterator
    // hub index for random walks
    int N; // #hubs
    int l; // #samples for each hub 
    double c; // the decay factor
    double lower_bound; // the lower bound utility of top k hubs
    DirectedG * g_ptr;
    vector<double> utility_array; // the utility array uility[i] is the utility of node i, used in pruning  node pairs in bprw, index are node ids
    SFMTRand rand_gen; // the random number generator
    Rw_Hubs(DirectedG &g, int _N, int _l, double c_ ):N(_N), l(_l),c(c_), g_ptr(&g){
        // init the vectors
        hubs.resize(g.n); // there are n slots in hubs
        this->utility_array.resize(g.n);
        this->hub_bits.resize(g.n);
    }
    ~Rw_Hubs(){
        cout << "destrying perfect hash..." << endl;
        cout << "exited Rw_Hubs" << endl;
    }
    int query_1s(const NodePair &np, int k); // query number 1s in the range of [0,k] of np
    bool query_single_pair(const NodePair &np); // query a termination node for a single node pair
    bool contains(NodePair &np);
    void select_hubs(); // select N hubs for random walks
    void sample_random_walks_for_hubs(); // samples random walks

    void build_hubs(){// the composition function for building index
        select_hubs();
        sample_random_walks_for_hubs();
    }


    // some statistic auxiliary variable
    int number_of_contains_queries=0;


    // index data
    Prefix_Sum pre_sum;
    vector<Single_Set> hubs;
    vector<Hub_Bit_Map> hub_bits; // the bit map for hubs 

};


inline unsigned int elegant_pair_f(NodePair & p){
    // the elegant pair function for mapping a node pair to an interger
    // assumption: p.first < p.second
    if(p.first >= p.second){
        return p.first * p.first + p.first + p.second;
    }else{
        return p.second * p.second + p.first;
    }
}

inline unsigned int elegant_pair_f(unsigned int & a, unsigned int & b){
    // the elegant pair function for mapping a node pair to an interger
    // assumption: p.first < p.second
    if( a >= b){
        return a * a + a + b;
    }else{
        return b * b + a;
    }
}


#endif
