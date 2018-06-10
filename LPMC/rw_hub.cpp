#include "rw_hub.h"
#include <climits>


bool Rw_Hubs::contains(NodePair &np){
    return (this->utility_array[np.first] * this->utility_array[np.second]) > lower_bound;
}

void Rw_Hubs::select_hubs(){ 
    // the top k method for hub seleciton O(nlogn+klogk)
    // select N random walks hubs for Graph g
    
    int n  = g_ptr->n;
    cout << n << endl;

    vector<pair<size_t, double>> utility(g_ptr->n); // utility for each node: node id, utility
    // fill in the utility table
    cout << "filling in the utility values..." << endl;
    for(int i = 0; i < g_ptr->n; i++){
        int out_start = g_ptr->off_out[i]; 
        int out_end = g_ptr->off_out[i+1];
        utility[i].first = i;
        for(int j = out_start; j < out_end; j++){
            auto out_neighbor = g_ptr->neighbors_out[j];
            int in_deg = g_ptr->in_degree(out_neighbor);
            if(in_deg > 0){
                utility[i].second += (1.0 / double(in_deg));
                this->utility_array[i] += (1.0 / double(in_deg));
            }
        }
    }
    cout << "fill utility finished..." << endl;
    // sort the index
    // for(auto & item: utility){
    //     cout << format("%s: %s") % item.first % item.second << endl;
    // }
    cout << "sorting the index" << endl;
    sort(utility.begin(), utility.end(), sort_hub_pred());
    cout << "sorting finished.." << endl;

    // for(auto & item: utility){
    //     cout << format("node: %s, utility: %s") % item.first % item.second << endl;
    // }


    // select hubs
    int k = 0; // select top k pairs with maximum utilities
    cout << "init the maximum heap... " << endl;
    std::priority_queue<Hub_Item> q;
    cout << "init heap finished.." << endl;
    int a = 0; // the cursor of the first dimension 
    int b = 1; // the cursor of the second dimension
    int i,j; // the node pair 
    std::priority_queue<Hub_Item> h;
    sparse_hash_set<NodePair> position_set; // the set to record the explored positions ([0,n-1], [0,n-1])
    h.push(utility_hub_converter(a,b,utility));
    position_set.insert(NodePair{a,b});
    // sparse_hash_map<unsigned int, pair<NodePair, pair<NodePair, NodePair>>> tmp; // original node pair, swaped node pair, index of this node pair
    while(k < N && !h.empty()){
        // cout << format("k is: %s" ) % k << endl;

        // cout << format("heap poping out heap size: %s") % h.size() << endl;
        Hub_Item hub = h.top(); //index for node pair in the position of utility vector
        a = hub.position.first;
        b = hub.position.second;
        // cout << format("before switch i: %s, j:%s") % i % j << endl;
        tie(i,j) = minmax(hub.np.first, hub.np.second);
        // cout << format("after switch i: %s, j:%s") % i % j << endl;

        NodePair hub_pair{i,j}; // we can ensure that i < j
        double hub_utility = hub.utility;
        h.pop();

        // pre_sum[hub_pair] = vector<int>(l);
        hubs[hub_pair.first].insert(hub_pair.second);


        // init the hub bits
        this->hub_bits[hub_pair.first][hub_pair.second].first.resize(this->l);
        this->hub_bits[hub_pair.first][hub_pair.second].second = 0;

        lower_bound = hub_utility; // update the lower bound

        // cout << format("k is: %s, utility: %s, hub: %s") % k % hub_utility % hub_pair << endl;



        if(((a+1) != b) && (a+1) < g_ptr->n){ // the down move
            if(!position_set.contains(NodePair{a+1,b})){
                i = utility[a+1].first;
                j = utility[b].first;
                NodePair current_np{i,j};
                double u = utility[a+1].second * utility[b].second;
                NodePair next_position{a+1,b};
                h.push(Hub_Item(current_np, u, next_position));
                position_set.insert(NodePair{a+1,b});
            }
        }
        if((b+1) < g_ptr->n){ // the right move 
            if(!position_set.contains(NodePair{a,b+1})){
                i = utility[a].first;
                j = utility[b+1].first;
                NodePair current_np{i,j};
                NodePair next_position{a,b+1};
                double u = utility[a].second * utility[b+1].second;
                h.push(Hub_Item(current_np, u, next_position));
                position_set.insert(NodePair{a,b+1});
            }
        }
        k ++;
    }


}


void Rw_Hubs::sample_random_walks_for_hubs(){
    // sample random walks for hubs
    // set up the uniform discret distribution 
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    std::geometric_distribution<int> geo_distribution(1-c);

    for(int j = 0; j < g_ptr->n; j++){
        for(auto & second: hubs[j]){
            NodePair np{j,second};
            // cout << format("%s: ") % np << endl;
            int number_of_meets = 0;
            for(int i = 0;i<l;i++){
                int length = 1 + geo_distribution(generator);
                int indicator = sample_an_1c_walk(np, (*g_ptr), length, rand_gen);
                if(indicator == 1){
                    number_of_meets++;
                    // pre_sum[np][i] = number_of_meets;
                    this->hub_bits[np.first][np.second].first[i] = true;
                    // cout << format("sample a meet ...in indexing at %s") % i << endl;
                }
                // cout << (*bitset_ptr)[i];
            }
        }
        // cout << endl;
        // cout << format("number of meets: %s") % number_of_meets << endl;
        // cout << format("size: %s, maximum size: %s") % (*bitset_ptr).size() % (*bitset_ptr).max_size() << endl;
        // cout << "=========================" << endl;
    }
    cout << "finished building random walks for hubs" << endl;
}


int Rw_Hubs::query_1s( const NodePair &np, int k){
    // assumption: np.first < np.second
    // assumption: np is already in the hub
    // boost::dynamic_bitset<> A(*hub_idx[np]);
    // A = A >> (l-k); // clear out the un-needed ones
    // return A.count();
    // cout << format("np: %s, k: %s, l:%s, size of this hub index: %s") % np % k  % this->l  % pre_sum[np].size() << endl;
    return pre_sum[np][k-1]; // return the sum of 1s of first k positions
}

bool Rw_Hubs::query_single_pair(const NodePair & np){
    pair<vector<bool>, size_t> & current_bit_map = this->hub_bits[np.first][np.second]; 
    bool result = current_bit_map.first[current_bit_map.second];
    current_bit_map.second = (current_bit_map.second + 1) % this->l;
    return result;
    // size_t & cursor = this->hub_bits[np.first][np.second].second;
    // // cout << format("cursor: is %s, number of samples per hub: %s") % cursor % this->l << endl;
    // cursor = (cursor+1) % this->l;
    // // cout << format("cursor: is %s, number of samples per hub: %s") % cursor % this->l << endl;
}




