#include "rw_hub.h"
#include <climits>

bool Rw_Hubs::contains(NodePair &np){
    return !(hub_idx.find(np) == hub_idx.end());
}

void Rw_Hubs::select_hubs(){
    // select N random walks hubs for Graph g
    DirectedG &g = *g_ptr; // use the pointer to init the reference
    
    int n  = num_vertices(g);
    cout << n << endl;
    vector<double> utility(n,0);

    // fill in the utility table
    for(int i = 0; i < n; i++){
        DirectedG::out_edge_iterator out_start, out_end, out_it;
        tie(out_start,out_end) = out_edges(i,g);
        for(out_it = out_start; out_it != out_end; out_it ++){
            auto current_out_neighbor = target(*out_it,g);
            int in_deg = in_degree(current_out_neighbor,g);
            if(in_deg > 0){
                utility[i] += (1.0 / double(in_deg));
            }
        }
    }

    // sort the utilities with index
	vector<size_t> sorted_index = sort_indexes(utility); // store the sorted index based on utility
    // for(int i = 0; i< n;i++){
    //     cout << format("%s: %s") % sorted_index[i] % utility[sorted_index[i]] << endl;
    // }


    // select hubs
    int k = N; // select top k pairs with maximum utilities
    max_unique_heap_for_hub h{g, utility, sorted_index}; // the heap for explored node pairs
    h.push({0,1});
    while(k >= 0){
        NodePair pair_index = h.pop(); //index for node pair in the position of utility vector
        int a = pair_index.first;
        int b = pair_index.second;

        // add to  hubs stores
        NodePair hub_pair{sorted_index[a],sorted_index[b]};

        hub_idx[hub_pair] = new dynamic_bitset<>(l); // create a bitset with l bits 


        if((a+1) != b){ // ignore the diagonol entries
            h.push({a+1,b});
        }
        h.push({a,b+1});

        k--;
    }

    // cout << "Selected Hubs: " << endl;
    // for(auto & hub:hubs){
    //     int a, b;
    //     tie(a,b) = hub;
    //     double u = utility[a] * utility[b];
    //     cout << format("%s: %s") % hub % u << endl;
    // }
}

void Rw_Hubs::sample_random_walks_for_hubs(){
    // sample random walks for hubs
    // set up the uniform discret distribution 
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    std::geometric_distribution<int> geo_distribution(1-c);

    for(auto& item: hub_idx){
        NodePair np{item.first};
        boost::dynamic_bitset<> * bitset_ptr = item.second;
        // cout << format("%s: ") % np << endl;
        int number_of_meets = 0;
        for(int i = 0;i<l;i++){
            int length = 1 + geo_distribution(generator);
            int indicator = sample_an_1c_walk(np, (*g_ptr), length);
            if(indicator == 1){
                number_of_meets++;
            }
            (*bitset_ptr)[i] = indicator;
            // cout << (*bitset_ptr)[i];
        }
        // cout << endl;
        // cout << format("number of meets: %s") % number_of_meets << endl;
        // cout << format("size: %s, maximum size: %s") % (*bitset_ptr).size() % (*bitset_ptr).max_size() << endl;
        // cout << "=========================" << endl;
    }
}

void max_unique_heap_for_hub::clear(){
    R.clear();
    heap.clear();
}

bool max_unique_heap_for_hub::empty() const {
    return heap.empty();
}

const heap_hub_item& max_unique_heap_for_hub::top(){
    return heap.top();
}

NodePair max_unique_heap_for_hub::pop(){
    heap_hub_item top_element = top(); // the top element
    R.erase(top_element.pair_index); // remove from hash table 
    heap.pop();
    // cout << "pop: " << top_element << " sum: " << sum << endl;
    return top_element.pair_index;

}

void max_unique_heap_for_hub::push(NodePair node_pair){
    // push value to node pairs
    if (R.contains(node_pair)) {
        // the key is already exists, so udpate its value
        // handle_t handle = R[node_pair];
        // // udpate sum
        // (*handle).residual += value;
        // heap.increase(handle);
        return; // keep the heap unique
    } else {
        heap_hub_item hd{node_pair, *g_ptr, *utility, *argidx};
        handle_t t = heap.push(hd);
        R[node_pair] = t;  // insert the handler to the hash map
    }
}

int Rw_Hubs::query_1s( const NodePair &np, int k){
    // assumption: np is already in the hub
    boost::dynamic_bitset<> A(*hub_idx[np]);
    A = A >> (l-k); // clear out the un-needed ones
    return A.count();
}



