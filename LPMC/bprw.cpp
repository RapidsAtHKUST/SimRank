#include "bprw.h"

void unique_max_heap::clear() {
    R.clear();
    heap.clear();
    sum = 0;
}

bool unique_max_heap::empty() const {
    return heap.empty();
}

const heap_data &unique_max_heap::top() {
    return heap.top();
}

heap_data unique_max_heap::pop() {
    heap_data top_element = top(); // the top element
    double r = top_element.residual;
    R.erase(top_element.np); // remove from hash table 
    heap.pop();
    sum -= r;
    // cout << "pop: " << top_element << " sum: " << sum << endl;
    return top_element;
}

void unique_max_heap::push(NodePair node_pair, double value) {
    // push value to node pairs
    if (R.contains(node_pair)) {
        // the key is already exists, so udpate its value
        handle_t handle = R[node_pair];
        // udpate sum
        (*handle).residual += value;
        heap.increase(handle);
    } else {
        heap_data hd{node_pair, value, *g_ptr};
        handle_t t = heap.push(hd);
        R[node_pair] = t;  // insert the handler to the hash map
    }
    sum += value;
}

size_t unique_max_heap::size() const {
    return heap.size();
}

std::ostream &operator<<(std::ostream &os, const BLPMC_Config& config) {
    os << format("Configuration, linear regression: %s, hub index: %s") % config.is_use_linear_regression_cost_estimation % config.is_use_hub_idx << endl;
    return os;
}

std::ostream &operator<<(std::ostream &os, const heap_data &obj) {
    os << format{"(%s,%s):%s"} % obj.np.first % obj.np.second % obj.residual;
    return os;
}

std::ostream &operator<<(std::ostream &os, const PushFeature &obj) {
    // os << format{"(%s,%s):%s"} % obj.np.first % obj.np.second % obj.residual;
    os << format("Local push feature: degree: %s, heap_size: %s") % obj.d % obj.H ;
    return os;
}

std::ostream &operator<<(std::ostream &os, const MCFeature &obj) {
    os << format("MC feature: r_sum: %s, heap_size: %s") % obj.r % obj.H;
    return os;
}


BackPush::BackPush(string g_name_, DirectedG &graph, double c_, double epsilon_, double delta_, BLPMC_Config config_):
    heap(graph), g_name(g_name_), g(&graph), c(c_), epsilon(epsilon_), fail_prob(delta_), config(config_){
        if(config.is_use_linear_regression_cost_estimation){ // build the training model
            this->build_cost_estimation_model();
        }
        if(config.is_use_hub_idx){ // build the hub index
            rw_hubs = new Rw_Hubs(graph, config.number_of_hubs, config.number_of_samples_per_hub,c);
            rw_hubs->build_hubs();
        }
}


size_t BackPush::number_of_walkers(double sum) {
    double r = pow(c * sum / epsilon, 2.0) * log(fail_prob / 2.0) / -2.0;
    return ceil(r);
}

double BackPush::keep_push_cost(unique_max_heap &heap) {
    const heap_data &top_element = heap.top();
    size_t d;
    d = (*g).in_degree(top_element.np.first) * (*g).in_degree(top_element.np.second);
    if(config.is_use_linear_regression_cost_estimation){
        PushFeature lpf(d,heap.size());
        MCFeature mcf(heap.sum - top_element.residual, heap.size()+d);

        vector<double> extracted_lp_feature; 
        lp_extract_feature(lpf, extracted_lp_feature);

        vector<double> extracted_mc_feature ; 
        mc_extract_feature(mcf,extracted_mc_feature);
        return lp_linearmodel->predict(extracted_lp_feature) + mc_linearmodel->predict(extracted_mc_feature);
    }else{
        return push_cost * log(heap.size()) * d + heap.size()  +  mc_cost * number_of_walkers(heap.sum - (1 - c) * top_element.residual) * ( 1+ 1 / (1 - c));
    }
}

double BackPush::change_to_MC_cost(unique_max_heap &heap) {
    if(config.is_use_linear_regression_cost_estimation){
        MCFeature mcf(heap.sum , heap.size());
        // cout << mcf << " ";
        vector<double> extracted_mc_feature; 
        mc_extract_feature(mcf, extracted_mc_feature);
        return mc_linearmodel->predict(extracted_mc_feature);
    }else{
        return mc_cost * number_of_walkers(heap.sum) * ( 1.0 + 1.0 / (1-c));
    }
}

bool BackPush::is_keep_on_push(unique_max_heap &hp, int number_of_current_lp_operations) {
    // decide whether to do local push
    const heap_data &top_element = heap.top();
    if(is_training){ // if is training, stop based on random local push numbers
        // cout << top_element << endl;
        auto a = top_element.np.first;
        auto b = top_element.np.second;
        double residual = top_element.residual;
        auto indeg_a = g->in_deg_arr[a];
        auto indeg_b = g->in_deg_arr[b];
        if ((number_of_current_lp_operations < this->maximum_lp_operations) && (indeg_a * indeg_b < 10000)){
            // only local push on smaller node pairs, whose in-degree is not very large
            return true;
        }{
            return false;
        }
    }else{ // doing the actual query
        if(top_element.np.first == top_element.np.second){
            return true;
        }
        int a,b;
        tie(a,b) = deg_np(top_element.np);
        if( a * b == 0){
            return true;
        }else{
            double lp_cost = keep_push_cost(hp);
            double mc_cost = change_to_MC_cost(hp);
            // cout << format("lp cost est: %s, mc cost est:%s.") % lp_cost % mc_cost<< endl;
            return  (top_element.np.first == top_element.np.second) ||  (lp_cost < mc_cost); // when singleton nodes, directly keep on push
        }
    }
}


pair<double, double> BackPush::backward_push(NodePair np, unique_max_heap &container) {
    // only in charge of local push
    // np: the starting pair
    // threshold: the threold for r_sum of local push, useless when using self-adaptive strategy
    double p = 0;
    container.clear();
    container.push(np, 1);
    // cout << format("inital required samples: %s ") % number_of_walkers(container.sum) << endl;

    // stat the cost of local push 
    int  number_of_local_push_operations = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    double cost = 0;
    std::chrono::duration<double> elapsed ;
    while (!container.empty() && ( is_keep_on_push(container, number_of_local_push_operations))) {
        // check whether to stop
        // cout << "----------" << endl;

        heap_data top_element = container.pop(); // pop and fetch the top element
        // cout << top_element << endl;
        auto a = top_element.np.first;
        auto b = top_element.np.second;
        double residual = top_element.residual;
        auto indeg_a = g->in_deg_arr[a];
        auto indeg_b = g->in_deg_arr[b];
        auto off_a_beg = g->off_in[a];
        auto off_a_end = g->off_in[a + 1];
        auto off_b_beg = g->off_in[b];
        auto off_b_end = g->off_in[b + 1];
        // cout << top_element << " sum: " << container.sum << endl;
        // cout << format("%s is pushed, in-degree:%s, priority:%s") % top_element.np %  (indeg_a * indeg_b) % \
            top_element.get_priority() << endl;
        if (a == b) { // current_pair is singleton
            p += residual;
            // cout << format("meet happends: %s") % top_element.np << endl;
            continue;
        } else { // non-singleton nodes
            auto batch_local_push_start = std::chrono::high_resolution_clock::now();
            // auto indeg_a = (*g).in_degree(a);
            // auto indeg_b = (*g).in_degree(b);
            // DirectedG::in_edge_iterator ina_it, ina_end;
            // DirectedG::in_edge_iterator inb_it, inb_end;
            // tie(ina_it, ina_end) = in_edges(a, *g);
            // no need to consider a*b==0, since it would not jump into the loops
            for (auto off_a = off_a_beg; off_a < off_a_end; off_a++) {
                auto in_nei_a = g->neighbors_in[off_a];
                for (auto off_b = off_b_beg; off_b < off_b_end; off_b++) {
                    auto in_nei_b = g->neighbors_in[off_b];

                    // cout << a << " " << b << " pushing to: " << in_nei_a << " " << in_nei_b << endl;
                        //  " ," << indeg_a << " ," << indeg_b << endl;

                    container.push(NodePair{in_nei_a, in_nei_b}, c * residual / (indeg_a * indeg_b));
                    number_of_local_push_operations += 1;
                }
            }
            // for (; ina_it != ina_end; ++ina_it) {
            //     auto ina = source(*ina_it, *g);
            //     tie(inb_it, inb_end) = in_edges(b, *g);
            //     for (; inb_it != inb_end; ++inb_it) {
            //         auto inb = source(*inb_it, *g);
            //         // cout << a << " " << b << " pushing to: " << ina << " " << inb << " ," << indeg_a << " ," << indeg_b << endl;


            //         container.push(NodePair{ina, inb}, c * residual / (indeg_a * indeg_b));
            //         number_of_local_push_operations += 1;



            //         // cost += log(heap.size()); // add the heap insertion cost 
            //     }
            // }
            auto batch_local_push_end = std::chrono::high_resolution_clock::now();
            if(heap.size() >= 1 && is_training){ // only count for heap size larger than 1, and is building linear model 
                elapsed = batch_local_push_end - batch_local_push_start;
                PushFeature fp(indeg_a * indeg_b, container.size());
                lp_data_X.push_back(fp); 
                lp_data_Y.push_back(elapsed.count()); 
                // cout << format("Node pair: %s, elapsed time cost: %s, heap size: %s") % NodePair{ina,inb} % elapsed.count() % heap.size() << endl;
                // update statics
                // cout << "constant factor for per local push " << single_lp_const << endl;
            }
        }

        // cout << format("current heap: ")  << endl;
        // for(auto & item: container.heap){
        //     cout << format("node pair: %s, residual: %s") % item.np % item.residual << endl;
        // }
        // cout << format("required number of walks: %s") % number_of_walkers(container.sum) << endl;
    }
    // cout << format("Total number of pushes: %s") % cost << endl;
    // cout << format{"deterministic estimate: %s, final residual sum: %s"} % p  % container.sum << endl;
    auto end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cost = elapsed.count();
    // if(!is_training){
    //     cout << "total number of lp operations " << number_of_local_push_operations << endl;
    // }
    return {p, cost};
}

double BackPush::random_bp(NodePair np, double rsum) {
    // backward push by random
    return 0;
}

double BackPush::query_one2one(NodePair np) { // pairwise SimRank estimation
    if (np.first == np.second) {
        return 1.0;
    }

    // cout << "======= greedy test ======" << endl;
    double p = backward_push(np, heap).first;
    // if(!is_training){
    //     cout << "push estimate: " << p << ", final residuals: " << heap.sum << endl;
    // }

    auto mc_start = std::chrono::high_resolution_clock::now();
    double mc_estimate = MC_random_walk(number_of_walkers(heap.sum)).first;
    auto mc_end = std::chrono::high_resolution_clock::now();

    if(is_training){ // only collect data while training
        std::chrono::duration<double> elapsed ;
        elapsed = mc_end - mc_start;
        MCFeature mf(heap.sum, heap.size());
        mc_data_X.push_back(mf);
        mc_data_Y.push_back(elapsed.count());
    }

    return p + mc_estimate;
}



pair <double, double> BackPush::MC_random_walk(int N) { // perform random walks based on current residuals in the heap
    // assume that there is no singleton nodes in current residuals
    // N: number of samples
    
    if(heap.empty()){ // corner case: heap is empty
        return {0,0};
    }

    // cout << format("require %s pairs of samples") % N << endl;
    if (N == 0) {
        return {0,0};
    }
    double mc_estimate = 0;

    // set up the uniform discret distribution 
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution(0.0, 1.0);


    // set up the CDF for weights of node pairs
    vector<double> weights;
    vector<NodePair> node_pairs;
    auto begin = heap.heap.begin();
    auto end = heap.heap.end();
    for (auto it = begin; it != end; ++it) {
        weights.push_back((*it).residual / heap.sum);
        node_pairs.push_back((*it).np);
    }
    constexpr int YCHE_MAX_INT = 1 << 30;
    vector<int> cdf(weights.size(), 0);
    auto prev = 0.0;
    auto accumulation = prev;
    for (auto i = 0; i < weights.size(); i++) {
        accumulation = prev + weights[i];
        cdf[i] = static_cast<int>(accumulation * YCHE_MAX_INT);
        prev = accumulation;
    }
    cdf.back() = YCHE_MAX_INT;

    // set up the geometry distribution
    std::geometric_distribution<int> geo_distribution(1-c);

    // helper set
    spp::sparse_hash_set<NodePair> starting_pairs;


    // begin sampling
    auto start_time = std::chrono::high_resolution_clock::now();
    double meeting_count = 0;

    vector<NodePair> starting_positions(N); // the starting positions 
    vector<int> length_of_rws(N); //and length of samples
    for(int i = 0; i< N;i++){
        int index = BinarySearchForGallopingSearchAVX2(&cdf.front(), 0, static_cast<uint32_t>(cdf.size()),
                                                       static_cast<int>(rand_gen.double_rand() * YCHE_MAX_INT)); // index for node pairs
        NodePair sampled_np = node_pairs[index];
        // int length_of_random_walk = 1 + floor(log(double(rand_gen.double_rand())) / log(c));
        int length_of_random_walk = 1;
        while(rand_gen.double_rand()<c){
            length_of_random_walk++;
        }
        starting_pairs.insert(sampled_np);
        starting_positions[i] = sampled_np;
        length_of_rws[i] = length_of_random_walk;
    }

    // for(auto& item: starting_pairs){
    //     cout << format("random walk from node pair: %s") % item << endl;
    // }

    if(this->is_use_hub()){ // MC sampling with hubs, focus on updating the meeting_count value
        meeting_count = this->sample_N_random_walks_with_hubs(starting_positions, length_of_rws);
    }else{ // normal sampling, forcus on updating meeting point value
        meeting_count = this->sample_N_random_walks(starting_positions, length_of_rws);
    }

    
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    double cost = elapsed.count();

    mc_estimate += c * heap.sum * (double(meeting_count) / double(N));
    // cout << format("total number of random walks: %s") % N << endl;
    return {mc_estimate, cost};

}

int BackPush::sample_N_random_walks_with_hubs(vector<NodePair> & nps, vector<int> & lengths){
    // return the number of meets of N random walks
    int N = nps.size();
    int meeting_count = 0;
    // aggreated query the hubs  
    // the auxiliary hash table for recording hub visits
    for(int i = 0; i< N;i++){
        int length_of_random_walk = lengths[i];
        NodePair sampled_np = nps[i];
        int sample_result = this->sample_one_pair_with_hubs(sampled_np,length_of_random_walk);
        meeting_count += sample_result;
    }
    return meeting_count;
} 

int BackPush::sample_N_random_walks(vector<NodePair> & nps, vector<int> & lengths){
    // normal sampling without index
    int N = nps.size();
    int meeting_count = 0;
    // aggreated query the hubs  
    // the auxiliary hash table for recording hub visits
    sparse_hash_map<NodePair, int> Q;
    for(int i = 0; i< N;i++){
        // auto start_time = std::chrono::high_resolution_clock::now();

        int length_of_random_walk = lengths[i];
        NodePair sampled_np = nps[i];
        int sample_result = this->sample_one_pair(sampled_np,length_of_random_walk);
        meeting_count += sample_result;

        // auto end_time = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double> elapsed = end_time - start_time;
        // cout << format("length of random walk: %s, time: %s") %  length_of_random_walk % (elapsed.count() * 1000) << endl;
    }
    return meeting_count;
}

int BackPush::sample_one_pair_with_hubs(NodePair sampled_np, int length_of_random_walk){
    // sample one pair of random walk sample one pair with hubs
        int a = sampled_np.first;
        int b = sampled_np.second;
        double indicator = 0;
        int step = 0; // 
        // bool is_long_path = length_of_random_walk >  (1.0 / (1.0-c)); // long path is those longer than expected 

        // test whether the starting node  a,b are in the hub or not
        if(rw_hubs->contains(a,b)){
            this->hub_hits ++;
            return rw_hubs->query_single_pair(NodePair{a,b});
        }


        while( step < length_of_random_walk && a != b){ // walk when > c or the first step
            a = sample_in_neighbor(a, *g, rand_gen);
            b = sample_in_neighbor(b, *g, rand_gen);
            step ++;
            if(a == -1 || b == -1){
                break;
            }else if (a == b){
                indicator = 1;
                break;
            }

            if(rw_hubs->contains(a,b)){
                this-> hub_hits ++;
                return rw_hubs->query_single_pair(NodePair{a,b});
            }

        }
        return indicator;
} 

int BackPush::sample_one_pair(NodePair np,  int length_of_random_walk) {
    // sample for one pair of random walk, with expected (1 + 1 / (1-c))
    // assume that a != b
    // if Q is not null, then update it for final hub aggregation
    int a = np.first;
    int b = np.second;
    double prob;
    double indicator = 0;
    int step = 0; // 
    while( step < length_of_random_walk && a != b){ // walk when > c or the first step
        a = sample_in_neighbor(a, *g, rand_gen);
        b = sample_in_neighbor(b, *g, rand_gen);


        step ++;
        if(a == -1 || b == -1){
            break;
        }else if (a == b){
            indicator = 1;
            break;
        }
    }
    return indicator;
}


pair<int,int> BackPush::deg_np(NodePair const &np){
    // a helper function extracts the degree of a node pair
    int a ,b;
    tie(a,b) = np;
    auto indeg_a = (*g).in_degree(a);
    auto indeg_b = (*g).in_degree(b);
    return {indeg_a,indeg_b};
}


void BackPush::build_cost_estimation_model(){
    // build the linear model for local push and mc cost
    cout << "building linear model for cost estimation..." << endl;
    is_training = true;
    lp_data_X.clear();
    lp_data_Y.clear();
    mc_data_X.clear();
    mc_data_Y.clear();
    int maximum_local_push_operations = 10000;
    
    cout << "collecting the data..." << endl;
    int number_of_vertices = (*g).n;
    for(int i = 0 ; i< data_size; i++){ // fill in total data: train + test
        double threshold = ceil(random_01() * double(maximum_local_push_operations)); // defined random stop criterior
        this->maximum_lp_operations = threshold;

        // generate random pair
        int a = random_int(0,number_of_vertices);
        int b = random_int(0,number_of_vertices); // the two nodes are not very far away from each other
        // cout << format("random pair: (%s,%s)") % a % b << endl;
        this->query_one2one(NodePair{a,b});
    }

    cout << "building the features..." << endl;
    // build feature for local push
    vector<vector<double>> lp_data_X_featured;
    for(auto &item:lp_data_X){
        vector<double> f ; 
        lp_extract_feature(item,f);

        lp_data_X_featured.push_back(f);
        // for(auto &feature:f){
        //     cout << feature << " ";
        // }
        // cout << endl;
    }

    // build feature for MC
    vector<vector<double>> mc_data_X_featured;
    for(auto &item: mc_data_X){
        vector<double> f ;
        mc_extract_feature(item,f); // single feature 

        mc_data_X_featured.push_back(f);

        // for(auto &feature:f){
        //     cout << feature << " ";
        // }
        // cout << endl;
    }


    cout << "spliting training and testing data..." << endl;
    /* split data for local push */
    int lp_data_size = lp_data_X_featured.size();
    int lp_train_size = 0.8 * lp_data_size;
    int lp_test_size  = 0.2 * lp_data_size;
    // split X
    vector<vector<double>> lp_test_X(lp_data_X_featured.end() - lp_test_size, lp_data_X_featured.end());
    vector<vector<double>> lp_train_X(lp_data_X_featured.begin(), lp_data_X_featured.begin() + lp_train_size);

    // split Y
    vector<double> lp_train_Y(lp_data_Y.begin(), lp_data_Y.begin() + lp_train_size);
    vector<double> lp_test_Y(lp_data_Y.end()- lp_test_size, lp_data_Y.end());

    /* split the data for Monte Carlo sampling */
    int mc_data_size = mc_data_X_featured.size();
    int mc_train_size = 0.8 * mc_data_size;
    int mc_test_size  = 0.2 * mc_data_size;
    // split X
    vector<vector<double>> mc_test_X(mc_data_X_featured.end() - mc_test_size, mc_data_X_featured.end());
    vector<vector<double>> mc_train_X(mc_data_X_featured.begin(), mc_data_X_featured.begin() + mc_train_size);

    // split Y
    vector<double> mc_train_Y(mc_data_Y.begin(), mc_data_Y.begin() + mc_train_size);
    vector<double> mc_test_Y(mc_data_Y.end()- mc_test_size, mc_data_Y.end());

    cout << "building the linear model for local push cost..." << endl;
    /* building model for LP */
    this->lp_linearmodel = new LinearRegression(lp_train_size, lp_train_X[0].size()); // the number of features is the size of the first row 
    lp_linearmodel->fit(lp_train_X, lp_train_Y);
    /* building model MC*/
    this->mc_linearmodel = new LinearRegression(mc_train_size, mc_train_X[0].size()); // the number of features is the size of the first row 
    mc_linearmodel->fit(mc_train_X, mc_train_Y);



    /* compute the accuracy for local push model*/
    double lp_train_sum_error = 0;
    for(int i = 0; i< lp_train_X.size();i++){
        double estimated = lp_linearmodel ->predict(lp_train_X[i]);
        double ground_truth = lp_train_Y[i];
        double error = ground_truth - estimated;
        lp_train_sum_error += abs(error);
        // cout << format("ground:%s, estimated: %s, error:%s") % ground_truth % estimated %  error << endl;
    }

    // cout << "testing the linear model for local push..." << endl;
    double lp_test_sum_error = 0;
    for(int i = 0; i< lp_test_X.size();i++){
        double estimated = lp_linearmodel ->predict(lp_test_X[i]);
        double ground_truth = lp_test_Y[i];
        double error = ground_truth - estimated;
        lp_test_sum_error += abs(error);
        // cout << format("ground:%s, estimated: %s, error:%s") % ground_truth % estimated %  error << endl;
    }
    cout << "Weight: " << endl;
    cout << lp_linearmodel->W << endl;
    cout << format("Average train error of local push: %s") % (lp_train_sum_error/ lp_train_size) << endl;
    cout << format("Average test error of local push: %s") % (lp_test_sum_error / lp_test_size) << endl;

    /* compute the accuracy for MC sampling model*/
    double mc_train_sum_error = 0;
    for(int i = 0; i< mc_train_X.size();i++){
        double estimated = mc_linearmodel ->predict(mc_train_X[i]);
        double ground_truth = mc_train_Y[i];
        double error = ground_truth - estimated;
        mc_train_sum_error += abs(error);
        // cout << format("ground:%s, estimated: %s, error:%s") % ground_truth % estimated %  error << endl;
    }

    // cout << "testing the linear model for local push..." << endl;
    double mc_test_sum_error = 0;
    for(int i = 0; i< mc_test_X.size();i++){
        double estimated = mc_linearmodel ->predict(mc_test_X[i]);
        double ground_truth = mc_test_Y[i];
        double error = ground_truth - estimated;
        mc_test_sum_error += abs(error);
        // cout << format("ground:%s, estimated: %s, error:%s") % ground_truth % estimated %  error << endl;
    }
    cout << "Weight: " << endl;
    cout << mc_linearmodel->W << endl;
    cout << format("Average train error of MC: %s") % (mc_train_sum_error/ mc_train_size) << endl;
    cout << format("Average test error of MC: %s") % (mc_test_sum_error / mc_test_size) << endl;
    

    // restore the state for query 
    is_training = false;
    rsum_threshold = 1;
}

void BackPush::lp_extract_feature(PushFeature &item, vector<double> &f){
    f.push_back(1);
    f.push_back(item.d);
    f.push_back((item.d + item.H) * log(double(item.d + item.H)) );
    f.push_back(item.H * log(double(item.H)));
    f.push_back(log(double(item.d + item.H)));
    f.push_back(log(double(item.H)));
    f.push_back(double(item.H));
}
void BackPush::mc_extract_feature(MCFeature & item, vector<double> &f){
    f.push_back(1);
    f.push_back(pow(item.r,2.0));
    f.push_back(item.H);
    if(item.H > 0){
        f.push_back(log(double(item.H)) * pow(item.r,2.0)); // for binary search in sampling nodes from H
    }else{
        f.push_back(0);
    }
}







