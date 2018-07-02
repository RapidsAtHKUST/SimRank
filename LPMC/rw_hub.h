#ifndef __RW_HUB_H__
#define __RW_HUB_H__


#include "stat.h"
#include <boost/dynamic_bitset.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/format.hpp> 

#include <util/graph_yche.h>
#include <util/sfmt_based_rand.h>
#include <util/sparse_matrix_utils.h> // the sparse hash map definition
#include <sparsehash/dense_hash_map>

// perfect hash
#include "phf.h"
#include "BooPHF.h"
#include "minimal_perfect_hash.h"

// bloom filter
#include <bf/all.hpp>

#include <roaring/roaring.hh>



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

struct Custome_HASH_NP{
     size_t operator()(NodePair const & np) const {
        size_t z;
        size_t x;
        size_t y;
        x = np.first;
        y = np.second;
        if(x > y){
            z = x * (x+1) + y;
        }else{
            z = y * y + x;
        }
        return std::hash<size_t>{}(z);
     }
};

struct Custome_EQ_NP{
    bool operator()( const NodePair& lhs, const NodePair& rhs ) const{
        return (lhs.first == rhs.first) && (lhs.second == rhs.second);
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
    int size = 0;
    bool is_empty = false;
    void init(vector<size_t> &  positions, size_t length_original_bitmap){
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
            size = this->next.size();
        }else{
            energy = -1; // indicates there is no 1s in this hub
            is_empty = true;
        }


    }
    int get(){
        //return 0/1 based on current energy and cursor
        // cout << format("is empty: %s, i: %s, energy: %s, size: %s ") \
        //     % is_empty % i % energy % size << endl;
        switch (energy){
            case -1: 
                return 0;
            case 0: 
                energy = next[i];
                i = (i + 1) % this->size;
                energy --;
                return 1;
            default:
                energy --;
                return 0;
        }
    }
};

struct Ball_Hub{ 
    // the ball based index, image 0s are white balls and 1s are black balls
    int L; // the sample size
    int number_of_1s ; // the number of 1s in L

    int current_num_1s; // number of unused 1s
    int remained_samples; // number of unused samples

    void init(int & L_, int & number_of_1s_){
        L = L_;
        number_of_1s = number_of_1s_;
    }
    int get(SFMTRand & rand_gen){
        // make sure there are samples to be used
        if(remained_samples == 0){
            // there is no samples to be used
            remained_samples = L;
            current_num_1s = number_of_1s;
        }

        int pos = rand_gen.uint_rand() % remained_samples;
        int result;
        if(pos < current_num_1s ){
            result = 1;
            current_num_1s --;
        }else{
            result = 0;
        }
        remained_samples --;
        return result;

    }
};


struct Rw_Hubs{ 
    // data structure of using BB Hash 

	typedef boomphf::SingleHashFunctor<u_int64_t> hasher_t; // for BBHash
	typedef boomphf::mphf<u_int64_t, hasher_t> boophf_t; // for BBHash
    boophf_t * bphf; // BB perfect hash function
    // vector<Distanct_1s> hub_vector_1s;
    vector<Ball_Hub> hub_vector_1s;
    vector<NodePair> hub_vector_np;
    vector<size_t> paired_keys;
    vector<unsigned int> rank; //rank[i]: the rank of node i w.r.t. utility

    typedef sparse_hash_map<NodePair, vector<int>> Prefix_Sum; // the prefix sum of 1s of of this node pair, size: N * K
    typedef sparse_hash_set<size_t> Single_Set; // just a set of integers for the right part of a hub

    // typedef sparse_hash_map<size_t, pair<boost::dynamic_bitset<> *, size_t>> Hub_Bit_Map; // key: the right part of a hub, value: bitmap, current iterator
    typedef sparse_hash_map<size_t, Distanct_1s *> Hub_Bit_Map; // key: the right part of a hub, value: bitmap, current iterator
    typedef google::dense_hash_map<NodePair, Distanct_1s *, Custome_HASH_NP, Custome_EQ_NP> HH_Table; // hub hash table

    // hub index for random walks
    int N; // #hubs
    int l; // #samples for each hub 
    int K; // the length of the upper triangular
    double c; // the decay factor
    double lower_bound; // the lower bound utility of top k hubs
    Roaring roaring_bitmap;  
    // vector<bool> exist_node; // the bitmap whether the node is in a hub
    boost::dynamic_bitset<unsigned int> exist_node;

    DirectedG * g_ptr;
    vector<double> utility_array; // the utility array uility[i] is the utility of node i, used in pruning  node pairs in bprw, index are node ids
    SFMTRand rand_gen; // the random number generator
    Rw_Hubs(DirectedG &g, int _N, int _l, double c_ ):l(_l),c(c_), g_ptr(&g){
        N = std::min(_N, int(g.n * (g.n - 1) / 2.0)); // the total number of hubs can not exceed the upper triangular
        cout << format("actual hub number: %s") % N << endl;
        // init the vectors
        this->K = comput_K(this->N);

        hubs.resize(g.n); // there are n slots in hubs
        this->utility_array.resize(g.n);
        this->rank.resize(g.n);
        this->hub_vector_1s.resize( K + K * (K-1) / 2); // total number of hubs
        this->exist_node.resize(g.n);
    }
    ~Rw_Hubs(){
        cout << "destrying perfect hash..." << endl;
        cout << "exited Rw_Hubs" << endl;
    }
    int query_1s(const NodePair &np, int k); // query number 1s in the range of [0,k] of np
    bool query_single_pair(const NodePair & np); // query a termination node for a single node pair
    bool contains(int& a, int& b){ // test whether both of node a and node b are in the hubs
        return a <= K && b<= K;
    }


    void select_top_N_hubs(); // select N hubs for random walks, fill in the node pair vector
    void select_top_K_hub_by_upper_triangular();
    vector<pair<size_t, double>> fill_and_rank_utility(); // return the array of utility({node id, utility})

    void sample_random_walks_for_hubs(); // samples random walks
    int comput_K(int); // compute the triangular given the hub size
    size_t get_index_of_hub_array(const NodePair & np);

    void build_hubs(){// the composition function for building index
        // select_top_N_hubs();
        // build_perfect_hash();
        select_top_K_hub_by_upper_triangular();
        sample_random_walks_for_hubs();
    }


    // some statistic auxiliary variable
    int number_of_contains_queries=0;
    int number_of_1s = 0;


    // index data
    Prefix_Sum pre_sum;
    vector<Single_Set> hubs;
    HH_Table* hht;

};



#endif
