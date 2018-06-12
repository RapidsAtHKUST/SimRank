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
#include "BooPHF.h"
#include "minimal_perfect_hash.h"



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

struct Distanct_1s{ // for querying samples
    vector<size_t> next; // store the distance between 1: next[i]
    size_t i = 0; // current cursor
    int energy = 0; // -1 if there no 1s in this hub
    Distanct_1s(vector<size_t> &  positions, size_t length_original_bitmap){
        // positions: the position array of 1s
        // construct the next array
        int n = positions.size();
        if(n > 0){ // the position matrix is not empty 
            next.resize(n);
            for(int k = 0; k < n - 1; k++){
                next[k] = positions[k+1] - positions[k];
            }
            next[n-1] = length_original_bitmap - (positions[n-1] - positions[0]);

            // initial position  
            energy = positions[0];
            i = 0;
        }else{
            energy = -1; // indicates there is no 1s in this hub
        }


    }
    int get(){
        //return 0/1 based on current energy and cursor
        if(energy >= 0){
            bool result = 0;
            if(energy == 0){ // we are at the position  of 1
                energy = next[i];
                i = (i + 1) % next.size();
                result = 1;
            }
            energy --;
            return result;
        }else{ // there no one 
            return 0;
        }
    }
};

struct Rw_Hubs{ 
    // the container for hubs
	typedef boomphf::SingleHashFunctor<u_int64_t>  hasher_t; // for BBHash
	typedef boomphf::mphf<u_int64_t, hasher_t> boophf_t; // for BBHash
    typedef sparse_hash_map<NodePair, vector<int>> Prefix_Sum; // the prefix sum of 1s of of this node pair, size: N * K
    typedef sparse_hash_set<size_t> Single_Set; // just a set of integers for the right part of a hub

    // typedef sparse_hash_map<size_t, pair<boost::dynamic_bitset<> *, size_t>> Hub_Bit_Map; // key: the right part of a hub, value: bitmap, current iterator
    typedef sparse_hash_map<size_t, Distanct_1s *> Hub_Bit_Map; // key: the right part of a hub, value: bitmap, current iterator

    typedef vector<pair<boost::dynamic_bitset<> *, size_t >> Second_Hub_Perfect_Bit; // the second leve of the index
    typedef vector<pair<Second_Hub_Perfect_Bit, minimal_perfect_hash::MinimalPerfectHash<unsigned int> *>> First_Hub_Perfect_Bit; // the first level of the index
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
        this->hub_perfect_bits.resize(g.n);
    }
    ~Rw_Hubs(){
        cout << "destrying perfect hash..." << endl;
        cout << "exited Rw_Hubs" << endl;
    }
    int query_1s(const NodePair &np, int k); // query number 1s in the range of [0,k] of np
    bool query_single_pair(const NodePair & np); // query a termination node for a single node pair
    bool contains(NodePair& np);
    void select_hubs(); // select N hubs for random walks
    void sample_random_walks_for_hubs(); // samples random walks

    void build_hubs(){// the composition function for building index
        select_hubs();
        // build_perfect_hash();
        sample_random_walks_for_hubs();
    }


    // some statistic auxiliary variable
    int number_of_contains_queries=0;
    int number_of_1s = 0;


    // index data
    Prefix_Sum pre_sum;
    vector<Single_Set> hubs;
    vector<Hub_Bit_Map> hub_bits; // the bit map for hubs 
    First_Hub_Perfect_Bit hub_perfect_bits; // the second level of the index is by perfect  hashing

    void build_perfect_hash(); // build perfect hash for each second level index

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
