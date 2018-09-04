// Backward Push based Random Walk
#ifndef __BPRW_H__
#define __BPRW_H__

#include <boost/heap/fibonacci_heap.hpp>
#include "stat.h"
#include "rw_hub.h"
#include "fgi.h"
#include <set>
#include <queue>
#include <algorithm>
#include <functional>
#include <exception>
#include <util/sfmt_based_rand.h>
#include <util/sparse_matrix_utils.h>
#include <util/search_yche.h>


using namespace boost::heap;

typedef GraphYche DirectedG;
typedef pair<int, double> QPair;

struct heap_data {
    // the data the heap maintains
    NodePair np;
    double residual;
    DirectedG *g_ptr; // the underlying graph
    heap_data(NodePair np_, double residual_, DirectedG &g) {
        np = np_;
        residual = residual_;
        g_ptr = &g;
    }
    heap_data(const heap_data & other){ // copy constructor
        np = other.np;
        residual = other.residual;
        g_ptr = other.g_ptr;
    }

    bool operator<(heap_data const &rhs) const { // doesn't change the memebr
        // return residual < rhs.residual;
        return get_priority() < rhs.get_priority();
    }

    double get_priority() const {
        auto indeg_a = (*g_ptr).in_degree(np.first);
        auto indeg_b = (*g_ptr).in_degree(np.second);
        auto total_indeg = indeg_a * indeg_b;
        if (total_indeg == 0 || np.first == np.second) {
            // no in-neighbor or singleton nodes 
            return std::numeric_limits<double>::max(); // the maximum priority
        } else {
            return residual / total_indeg;
        }
    }

    std::ostream &operator<<(std::ostream &os) {
        os << format{"(%s,%s): %s"} % np.first % np.second % residual;
        return os;
    }
};

struct data_item {
    NodePair np;
    double residual;

    data_item() {}

    data_item(NodePair np_, double r_) : np(np_), residual(r_) {}
};



struct unique_max_heap { // the heap structure used in BPRW
    typedef sparse_hash_map<NodePair, fibonacci_heap<heap_data>::handle_type> RMap; // hash table for residuals
    typedef fibonacci_heap<heap_data>::handle_type handle_t;
    RMap R;
    double sum;
    fibonacci_heap<heap_data> heap;
    DirectedG *g_ptr;

    unique_max_heap(DirectedG &g) : g_ptr(&g) {
        sum = 0;
    }

    const heap_data &top();

    heap_data pop();

    void clear();

    bool empty() const;

    void push(NodePair, double); // push value to the node pair
    size_t size() const;
};



std::ostream &operator<<(std::ostream &os, const heap_data &obj);

std::ostream &operator<<(std::ostream &os, const data_item &obj);

struct PushFeature {
    int d;
    int H;
    PushFeature(int d_, int H_size_){
        d = d_;
        H = H_size_;
    }
};
struct MCFeature {
    double r; // sum of residuals
    int H; // the heap size
    MCFeature(double r_, int H_size_){
        r = r_;
        H = H_size_;
    }
};

std::ostream &operator<<(std::ostream &os, const PushFeature &obj);

std::ostream &operator<<(std::ostream &os, const MCFeature &obj);

struct BLPMC_Config{
    bool is_use_linear_regression_cost_estimation = false;
    bool is_use_hub_idx = false;
    bool is_use_fg_idx = false;
    int number_of_hubs = 0 ;
    int number_of_samples_per_hub = 0;
    int number_of_trees = 0;

    BLPMC_Config(bool a, bool b, bool c){ // normal constructor
        is_use_linear_regression_cost_estimation = a;
        is_use_hub_idx = b;
        is_use_fg_idx = c;
    }
    BLPMC_Config(const BLPMC_Config &other){ // copy constructor
        is_use_linear_regression_cost_estimation = other.is_use_linear_regression_cost_estimation;
        is_use_hub_idx = other.is_use_hub_idx;
        is_use_fg_idx = other.is_use_fg_idx;
        number_of_hubs = other.number_of_hubs;
        number_of_samples_per_hub = other.number_of_samples_per_hub;
        number_of_trees = other.number_of_trees;
    }

    BLPMC_Config(){ // default constructor
        is_use_linear_regression_cost_estimation = false;
        is_use_hub_idx = false;
        is_use_fg_idx = false;
        number_of_hubs = 0 ;
        number_of_samples_per_hub = 0;
        number_of_trees = 0;
    }
};

std::ostream &operator<<(std::ostream &os, const BLPMC_Config& config);


struct BackPush { // Backward Push and MC sampling method for SimRank estimation
    string g_name;
    DirectedG *g; // pointer to the underlying graph
    double c;
    double epsilon; // error bound
    double fail_prob; // delta
    unique_max_heap heap; // the heap contains residuals
    // public methods
    SFMTRand rand_gen;
    BLPMC_Config config;

    BackPush(string g_name_, DirectedG &graph, double c_, double epsilon_, double delta_, BLPMC_Config config = BLPMC_Config());

    typedef pair<double, pair<double,double>> triple;

    pair<double, double>
    backward_push(NodePair np, unique_max_heap &); // self-adaptive backward local push, return estimate

    pair<int,int> deg_np(NodePair const & np); // extract the degree of node pairs
    

    // constant for cost estimation
    int push_cost = 1; // the contant for push operation given a current heap and d, which is degree of neighbors pushing to
    int mc_cost = 1; // the constant for MC sampling givn a double, which is the sum of heap residuals

    double random_bp(NodePair np, double rsum); // backward local push by random
    pair<double, double> MC_random_walk(int N); // perform random walks based on current residuals in the heap
    double query_one2one(NodePair np); // query single-pair SimRank scores
    int sample_one_pair(NodePair np, int length_of_random_walk); // sample one pair of random walk
    double keep_push_cost(unique_max_heap &heap); // compute the cost is we push one-step further
    double change_to_MC_cost(unique_max_heap &heap); // compute the cost if we turn to random walk  
    size_t number_of_walkers(double sum); // compute the number of random walkers, given current sum
    bool is_keep_on_push(
            unique_max_heap &heap, int number_of_current_lp_operations); // the indicator bool function to decide whether the local push would continue

    // methods of  similarity search for sets
    // PairHashMap query_set_scores(vector<NodePair> query_set); // estimate simrank scores of a set
    // PairSet top_k_in_set(vector<NodePair> query_set); // extract the top-k elements of a set of node pairs 

    /* linear regression for cost estimation */
    vector<PushFeature> lp_data_X; // the local push data set
    vector<double> lp_data_Y; // 
    vector<MCFeature> mc_data_X; // the MC data set
    vector<double> mc_data_Y; // 
    int data_size = 1000;
    double rsum_threshold = 1;
    int maximum_lp_operations = 50000; // the maximum number of local push operations
    bool is_training = false; // indicating whether training the linear model 
    LinearRegression * lp_linearmodel;
    LinearRegression * mc_linearmodel;
    void build_cost_estimation_model(); // build the linear regression model on-line

    void lp_extract_feature(PushFeature &, vector<double>& f); // extract feature 
    void mc_extract_feature(MCFeature &, vector<double>& f);

    vector<int> klist;
    void init_klist(int klength);

    // related for hub index
    Rw_Hubs * rw_hubs = NULL; // the pointer to the hub index
    int sample_one_pair_with_hubs(NodePair np, int length_of_random_walk); // sample one pair of random walk
    int sample_one_pair_with_fg(int x, int y, int tree_id);
    bool is_use_hub(){ // the criterior to use hub indices, only config is set is not enough, hub index must bt no Null
        if(is_training){ // do not hub while training the cost model 
            return false;
        }else{
            return config.is_use_hub_idx;
        }
    }
    bool is_use_fg() {
        if (is_training) {
            return false;
        } else {
            return config.is_use_fg_idx;
        }
    }
    int hub_hits = 0; // statistical information: #hits of hub
    int sample_N_random_walks(vector<NodePair> &, vector<int> &); // return the number of meets of N random walks
    int sample_N_random_walks_with_hubs(vector<NodePair> &, vector<int> &); // return the number of meets of N random walks
    int sample_N_random_walks_with_fg(vector<NodePair> &, vector<int> &);

    FG_Index *fg_idx = NULL;
    
    // the Top-K interface
    vector<QPair> top_k_sort(vector<NodePair>& Q, int k);
    vector<QPair> top_k_heap(vector<NodePair>& Q, int k); // use priority queue
    vector<QPair> top_k(vector<NodePair>& Q, int k); // return the top-k

    //pair<int, int> sample_N_random_walks_topk(vector<NodePair> &nps, vector<int> &lengths);
    void rw_init(unique_max_heap &bheap, vector<NodePair> &node_pairs, vector<int> &cdf);
    int MC_random_walk_topk(int N, vector<NodePair> &node_pairs, vector<int> &cdf, int nt);
    int sample_N_random_walks_with_hubs_topk(vector<NodePair> &nps);
    int sample_N_random_walks_with_fg_topk(vector<NodePair> &nps, int nt);
    int sample_N_random_walks_topk(vector<NodePair> &nps);
};


#endif
