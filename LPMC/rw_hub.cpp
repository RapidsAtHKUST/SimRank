#include "rw_hub.h"
#include <climits>



int Rw_Hubs::comput_K(int N){
    // compute the length of the edge of upper triangular
    int k =  ceil((sqrt(1.0 + 8 * N) - 1) / 2.0);
    return k;
}

vector<pair<size_t, double>> Rw_Hubs::fill_and_rank_utility(){
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
    cout << "sorting the index" << endl;
    sort(utility.begin(), utility.end(), sort_hub_pred());

    // fill in the rank
    cout << "filling in the rank array" << endl;
    for(int i = 0; i< n;i++){
        this->rank[utility[i].first] = i;
    }

    // rank of each node
    for(int i = 0; i< n;i++){
        if(i != rank[i]){
            cout << format("node: %s, rank: %s, %s %s") % i % this->rank[i] \
                % utility_array[i] % utility[rank[i]].second << endl;
        }
    }
    return  utility;
}

void Rw_Hubs::select_top_K_hub_by_upper_triangular(){
    // assume the node ids are already sorted by their utility
    cout << "filling in the hub vector" << endl;
    int number_hubs = 0;
    for(int i = 0; i< K; i++){
        for(int j = i+1; j <= K; j++){
            NodePair np{i,j};
            this->hub_vector_np.push_back(np);
            number_hubs ++;

        }
    }
    // reset number of hubs;
    this->N = number_hubs;

	// if your bitmaps have long runs, you can compress them by calling
	// run_optimize
	uint32_t size = roaring_bitmap.getSizeInBytes();
	roaring_bitmap.runOptimize();

	// you can enable "copy-on-write" for fast and shallow copies
	roaring_bitmap.setCopyOnWrite(true);


	uint32_t compact_size = roaring_bitmap.getSizeInBytes();
	std::cout << "size before run optimize " << size << " bytes, and after "
				<<  compact_size << " bytes." << std::endl;
}

size_t Rw_Hubs::get_index_of_hub_array(const NodePair &np){
    //np: any node pair
    //assumption: node id is its rank
    int i = np.first;
    int j = np.second;
    if(i > j){
        std::swap(i,j);
    }
    size_t idx = i + j * (j-1) / 2;
    return idx;
}


void Rw_Hubs::select_top_N_hubs(){ 
    // the top k method for hub seleciton O(nlogn+klogk)
    // select N random walks hubs for Graph g
    
    auto utility = this->fill_and_rank_utility();

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
        // hubs[hub_pair.first].insert(hub_pair.second);
        this->hub_vector_np.push_back(hub_pair); // vector of hub pairs


        lower_bound = hub_utility; // update the lower bound




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
    cout << "sampling random walks for hubs " << endl;
    for(auto& np : this->hub_vector_np){
        // size_t x = Custome_HASH_NP{}(np);
        // auto hashed_key = bphf->lookup(x);
        // // cout << format("sampling for np: %s, paired key: %s,  the hased key: %s") % np % x % hashed_key << endl;

        vector<size_t> position_of_1s;
        int num_pos = 0;
        // cout << format("sampling for %s: ") % np << endl;
        for(int i = 0;i<l;i++){
            int length = 1;
            while(rand_gen.double_rand()<c){
                length++;
            }
            int indicator = sample_an_1c_walk(np, (*g_ptr), length, rand_gen);
            if(indicator == 1){
                this->number_of_1s ++;
                num_pos ++;
                position_of_1s.push_back(i); // fill in the position matrix
                
            }
        }
        // cout << format("np: %s, position size: %s") % np % position_of_1s.size() << endl;
        // hub_vector_1s[get_index_of_hub_array(np)].init(position_of_1s, this->l);
        hub_vector_1s[get_index_of_hub_array(np)].init(this->l, num_pos);

        // (*hht)[np] = new Distanct_1s(position_of_1s, this->l);

    }

    cout << "finished building random walks for hubs" << endl;
}


int Rw_Hubs::query_1s( const NodePair &np, int k){
    return pre_sum[np][k-1]; // return the sum of 1s of first k positions
}

bool Rw_Hubs::query_single_pair(const NodePair& np){
    // cout << format("np: %s, index: %s, N: %s, k: %s") % np  \
    //     % get_index_of_hub_array(np) % this->N % this->K << endl;
    return hub_vector_1s[get_index_of_hub_array(np)].get(rand_gen);
    // return (*hht)[np]->get();
}




