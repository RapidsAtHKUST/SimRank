// Backward Push based Random Walk
#ifndef __BPRW_H__
#define __BPRW_H__

#include <random>

#include <boost/heap/fibonacci_heap.hpp>
#include <boost/format.hpp>

#include <sparsepp/spp.h>
#include <unordered_map>

#include "../util/graph_yche.h"
#include "../util/sfmt_based_rand.h"
#include "../util/sparse_matrix_utils.h"

using namespace boost::heap;

using spp::sparse_hash_map;
using boost::format;

using NodePair= pair<unsigned int, unsigned int>;

// used in the heap node
extern GraphYche *g_ptr;

struct heap_data {
    // the data the heap maintains
    NodePair np;
    double residual;

    heap_data(NodePair np_, double residual_) {
        np = np_;
        residual = residual_;
    }

    bool operator<(heap_data const &rhs) const { // doesn't change the memebr
        // return residual < rhs.residual;
        return get_priority() < rhs.get_priority();
    }

    double get_priority() const {
        auto indeg_a = g_ptr->in_deg_arr[np.first];
        auto indeg_b = g_ptr->in_deg_arr[np.second];
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

    data_item() = default;

    data_item(NodePair np_, double r_) : np(np_), residual(r_) {}
};

class Residual_Container { // an abstract container that holds residuals
public:
    double sum;

    virtual data_item pop() = 0;

    virtual void clear() = 0;

    virtual bool empty() const = 0;

    virtual void push(NodePair, double) = 0; // push value to the node pair
    virtual size_t size() const = 0;
};

class unique_max_heap : public Residual_Container { // the heap structure used in BPRW
public:
#ifdef SPARSE_HASH_MAP_FOR_HEAP
    typedef sparse_hash_map<NodePair, fibonacci_heap<heap_data>::handle_type> RMap; // hash table for residuals
#else
    typedef std::unordered_map<NodePair, fibonacci_heap<heap_data>::handle_type> RMap; // hash table for residuals
#endif
    typedef fibonacci_heap<heap_data>::handle_type handle_t;
    RMap R;
    fibonacci_heap<heap_data> heap;
    GraphYche *g_ptr;

    unique_max_heap(GraphYche &g) : g_ptr(&g) {
        sum = 0;
    }

    const heap_data &top();

    data_item pop();

    void clear();

    bool empty() const;

    void push(NodePair, double); // push value to the node pair
    size_t size() const;
};

std::ostream &operator<<(std::ostream &os, const heap_data &obj);

std::ostream &operator<<(std::ostream &os, const data_item &obj);

struct BackPush { // Backward Push
    string g_name;
    GraphYche *g; // pointer to the underlying graph
    double c;
    double epsilon; // error bound
    double fail_prob; // delta
    unique_max_heap heap; // the heap contains residuals
//    residual_set set_residual; // the set containing residuals for random push

#ifdef SFMT
    SFMTRand rand_gen;
#endif
public:
    int push_cost = 5; // the contant for push operation given a current heap and d, which is degree of neighbors pushing to
    int mc_cost = 1; // the constant for MC sampling givn a double, which is the sum of heap residuals
public:
    // public methods
    BackPush(string g_name_, GraphYche &graph, double c_, double epsilon_, double delta_);

    // self-adaptive backward local push, return estimate
    pair<double, int> backward_push(NodePair np, unique_max_heap &);

//    double MC_random_walk(); // perform random walks based on current residuals in the heap
    double MC_random_walk(int N);

    double query_one2one(NodePair np); // query single-pair SimRank scores

#if !defined(SFMT)
    double sample_one_pair(NodePair np, std::default_random_engine &generator,
                        std::uniform_real_distribution<double> &dist); // sample one pair of random walk
#else

    double sample_one_pair(NodePair np);

#endif

    double keep_push_cost(unique_max_heap &heap); // compute the cost is we push one-step further
    double change_to_MC_cost(unique_max_heap &heap); // compute the cost if we turn to random walk  
    size_t number_of_walkers(double sum); // compute the number of random walkers, given current sum

    // the indicator bool function to decide whether the local push would continue
    bool is_keep_on_push(unique_max_heap &heap);
};

#endif

