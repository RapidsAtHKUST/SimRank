// Backward Push based Random Walk
#ifndef __BPRW_H__
#define __BPRW_H__

#include <boost/heap/fibonacci_heap.hpp>

#include "graph.h"

using namespace boost::heap;

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

    bool operator<(heap_data const &rhs) const { // doesn't change the memebr
        // return residual < rhs.residual;
        return get_priority() < rhs.get_priority();
    }

    double get_priority() const {
        auto indeg_a = in_degree(np.first, *g_ptr);
        auto indeg_b = in_degree(np.second, *g_ptr);
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
    typedef sparse_hash_map<NodePair, fibonacci_heap<heap_data>::handle_type> RMap; // hash table for residuals
    typedef fibonacci_heap<heap_data>::handle_type handle_t;
    RMap R;
    fibonacci_heap<heap_data> heap;
    DirectedG *g_ptr;

    unique_max_heap(DirectedG &g) : g_ptr(&g) {
        sum = 0;
    }

    const heap_data &top();

    data_item pop();

    void clear();

    bool empty() const;

    void push(NodePair, double); // push value to the node pair
    size_t size() const;
};


struct residual_set : public Residual_Container { // the set structure which holds the residual data
public:
    // public members
    sparse_hash_map<NodePair, data_item> hash_d; // store the actually data
    // public methods
    residual_set() {
        sum = 0;
    }

    data_item pop(); // random pop an element
    void clear();

    bool empty() const;

    void push(NodePair, double); // push value to the node pair
    size_t size() const;
};

std::ostream &operator<<(std::ostream &os, const heap_data &obj);

std::ostream &operator<<(std::ostream &os, const data_item &obj);


struct BackPush { // Backward Push
    string g_name;
    DirectedG *g; // pointer to the underlying graph
    double c;
    double epsilon; // error bound
    double fail_prob; // delta
    unique_max_heap heap; // the heap contains residuals
    residual_set set_residual; // the set containing residuals for random push
    // public methods
    BackPush(string g_name_, DirectedG &graph, double c_, double epsilon_, double delta_);

    pair<double, int>
    backward_push(NodePair np, unique_max_heap &); // self-adaptive backward local push, return estimate

    double random_bp(NodePair np, double rsum); // backward local push by random
    double MC_random_walk(); // perform random walks based on current residuals in the heap
    double query_one2one(NodePair np); // query single-pair SimRank scores
    int sample_one_pair(NodePair np, std::default_random_engine &generator,
                        std::uniform_real_distribution<double> &dist); // sample one pair of random walk
    double keep_push_cost(unique_max_heap &heap); // compute the cost is we push one-step further
    double change_to_MC_cost(unique_max_heap &heap); // compute the cost if we turn to random walk  
    size_t number_of_walkers(double sum); // compute the number of random walkers, given current sum
    bool is_keep_on_push(
            unique_max_heap &heap); // the indicator bool function to decide whether the local push would continue

};

#endif
