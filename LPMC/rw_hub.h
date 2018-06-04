#ifndef __RW_HUB_H__
#define __RW_HUB_H__


#include "stat.h"
#include "graph.h"
#include <boost/dynamic_bitset.hpp>
#include <boost/heap/fibonacci_heap.hpp>

using namespace boost;
using namespace boost::heap;

struct heap_hub_item{
    // the item data for hub selection for heap
    NodePair pair_index; // the pair of node index in the argsort
    DirectedG *g_ptr; // the pointer to the underlying graph
    vector<double> * utility; // the reference to the utility vector
    vector<size_t> * argidx; // the reference to the arg index 
    heap_hub_item(NodePair np_, DirectedG &g, vector<double>& utility_, 
            vector<size_t> & sorted_index) {
        pair_index = np_;
        g_ptr = &g;
        utility = &utility_;
        argidx = &sorted_index;
    }

    bool operator<(heap_hub_item const &rhs) const { // doesn't change the memebr
        // return residual < rhs.residual;
        return get_priority() < rhs.get_priority();
    }

    double get_priority() const {
        int a = (*argidx)[pair_index.first];
        int b  = (*argidx)[pair_index.second];
        return (*utility)[a] * (*utility)[b];
    }

    std::ostream &operator<<(std::ostream &os) {
        os << format{"(%s,%s): %s"} % pair_index.first % pair_index.second % get_priority();
        return os;
    }
};

struct max_unique_heap_for_hub { // the heap structure used in BPRW
    typedef sparse_hash_map<NodePair, fibonacci_heap<heap_hub_item>::handle_type> RMap; // hash table for residuals
    typedef fibonacci_heap<heap_hub_item>::handle_type handle_t;
    RMap R;
    fibonacci_heap<heap_hub_item> heap;
    DirectedG *g_ptr;
    vector<double> * utility; // the pointer to the utility array
    vector<size_t> * argidx; // the reference to the arg index 

    max_unique_heap_for_hub(DirectedG &g, vector<double>& utility_, vector<size_t>& argidx_) : 
        g_ptr(&g), utility(&utility_), argidx(&argidx_){
    }

    const heap_hub_item &top();

    NodePair pop(); // only needs to pop out the node pair with the maximum utility

    void clear();

    bool empty() const;

    void push(NodePair); // push value to the node pair
    size_t size() const;
};


struct Rw_Hubs{ 
    // the container for hubs
    typedef sparse_hash_map<NodePair, boost::dynamic_bitset<> *, hash_duplicate, hash_duplicate_equal> Hub_Idx;
    // hub index for random walks
    int N; // #hubs
    int l; // #samples for each hub 
    Hub_Idx hub_idx;
    sparse_hash_map<NodePair, int> count; // counting map which stores 
    double c; // the decay factor
    DirectedG * g_ptr;
    Rw_Hubs(DirectedG &g, int _N, int _l, double c_ ):N(_N), l(_l),c(c_), g_ptr(&g){
    }
    int query_1s(const NodePair &np, int k); // query number 1s in the range of [0,k] of np
    bool contains(NodePair &np);
    void select_hubs(); // select N hubs for random walks
    void sample_random_walks_for_hubs(); // samples random walks

    void build_hubs(){// the composition function for building index
        select_hubs();
        sample_random_walks_for_hubs();
    }
};



#endif
