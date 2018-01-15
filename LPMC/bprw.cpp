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

data_item unique_max_heap::pop() {
    heap_data top_element = top(); // the top element
    double r = top_element.residual;
    R.erase(top_element.np); // remove from hash table 
    heap.pop();
    sum -= r;
    // cout << "pop: " << top_element << " sum: " << sum << endl;
    return data_item{top_element.np, top_element.residual};
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

std::ostream &operator<<(std::ostream &os, const heap_data &obj) {
    os << format{"(%s,%s):%s"} % obj.np.first % obj.np.second % obj.residual;
    return os;
}

std::ostream &operator<<(std::ostream &os, const data_item &obj) {
    os << format{"(%s,%s):%s"} % obj.np.first % obj.np.second % obj.residual;
    return os;
}

BackPush::BackPush(string g_name_, DirectedG &graph, double c_, double epsilon_, double delta_):
        heap(graph), g_name(g_name_), g(&graph), c(c_), epsilon(epsilon_), fail_prob(delta_) {
}

size_t BackPush::number_of_walkers(double sum) {
    double r = pow(c * sum / epsilon, 2.0) * log(fail_prob / 2.0) / -2.0;
    return ceil(r);
}

double BackPush::keep_push_cost(unique_max_heap &heap) {
    const heap_data &top_element = heap.top();
    size_t d;
    d = in_degree(top_element.np.first, *g) * in_degree(top_element.np.second, *g);
    return push_cost * log(heap.size()) * d +  mc_cost * number_of_walkers(heap.sum - (1 - c) * top_element.residual) * ( 1+ 1 / (1 - c));
}

double BackPush::change_to_MC_cost(unique_max_heap &heap) {
    return mc_cost * number_of_walkers(heap.sum) * ( 1.0 + 1.0 / (1-c));
}

bool BackPush::is_keep_on_push(unique_max_heap &hp) {
    const heap_data &top_element = heap.top();
    return  (top_element.np.first == top_element.np.second) || (keep_push_cost(hp) < change_to_MC_cost(hp) ); // when singleton nodes, directly keep on push
}

pair<double, int> BackPush::backward_push(NodePair np, unique_max_heap &container) {
    // only in charge of local push
    // np: the starting pair
    int cost = 0; // record the number of pushes as cost
    double p = 0;
    container.clear();
    container.push(np, 1);
    while (!container.empty() && is_keep_on_push(container)) {
        // check whether to stop

        auto top_element = container.pop(); // pop and fetch the top element
        // cout << top_element << endl;
        auto a = top_element.np.first;
        auto b = top_element.np.second;
        double residual = top_element.residual;
        // cout << top_element << " sum: " << container.sum << endl;
        if (a == b) { // current_pair is singleton
            p += residual;
            continue;
        } else { // non-singleton nodes
            auto indeg_a = in_degree(a, *g);
            auto indeg_b = in_degree(b, *g);
            DirectedG::in_edge_iterator ina_it, ina_end;
            DirectedG::in_edge_iterator inb_it, inb_end;
            tie(ina_it, ina_end) = in_edges(a, *g);
            for (; ina_it != ina_end; ++ina_it) {
                auto ina = source(*ina_it, *g);
                tie(inb_it, inb_end) = in_edges(b, *g);
                for (; inb_it != inb_end; ++inb_it) {
                    auto inb = source(*inb_it, *g);
#ifdef DEBUG
                    cout << a << " " << b << " pushing to: " << ina << " " << inb << " ," << indeg_a << " ," << indeg_b
                         << endl;
#endif
                    container.push(NodePair{ina, inb}, c * residual / (indeg_a * indeg_b));
                    ++cost;
                }
            }
        }
    }
    // cout << format("Total number of pushes: %s") % cost << endl;
    // cout << format{"deterministic estimate: %s, final residual sum: %s"} % p  % container.sum << endl;
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
    double r_sum = 0.6;
    // cout << "======= random test ======" << endl;
    // backward_push(np, r_sum, set_residual);

    cout << "======= greedy test ======" << endl;
    double p = backward_push(np, heap).first;
    cout << "push estimate: " << p << ", final residuals: " << heap.sum << endl;

    double mc_estimate = MC_random_walk(number_of_walkers(heap.sum));

    return p + mc_estimate;
}

double BackPush::MC_random_walk(int N) { // perform random walks based on current residuals in the heap
    // assume that there is no singleton nodes in current residuals
    // N: number of samples
    
    if(heap.empty()){ // corner case: heap is empty
        return 0;
    }

    cout << format("require %s pairs of samples") % N << endl;
    if (N == 0) {
        return 0;
    }
    double mc_estimate = 0;

    // set up the discret distribution 
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    vector<double> weights;
    vector<NodePair> node_pairs;
    auto begin = heap.heap.begin();
    auto end = heap.heap.end();
    for(auto it = begin; it !=end; ++ it){
        weights.push_back((*it).residual / heap.sum);
        node_pairs.push_back((*it).np);
    }
    std::discrete_distribution<int> residuals_dist (weights.begin(),weights.end());

    // set up the geometry distribution
    std::geometric_distribution<int> geo_distribution(1-c);

    // begin sampling
    double meeting_count = 0;
    for(int i = 0; i< N; i++){
        int index =  residuals_dist(generator) ; // index for node pairs
        int length_of_random_walk = geo_distribution(generator) + 1;
        NodePair sampled_np = node_pairs[index];
        if(sampled_np.first == sampled_np.second){
            meeting_count += 1.0;
        }else{
            double sample_result= sample_one_pair(sampled_np, generator, distribution, length_of_random_walk);
            meeting_count += sample_result;
        }
    }

    mc_estimate += heap.sum * (meeting_count / double(N));
    return mc_estimate;
}

double BackPush::sample_one_pair(NodePair np, std::default_random_engine &generator,
                              std::uniform_real_distribution<double> &distribution, int length_of_random_walk) {
    // sample for one pair of random walk, with expected (1 + 1 / (1-c))
    // assume that a != b
    int a = np.first;
    int b = np.second;
    double prob;
    double indicator = 0;
    int step = 0; // 


    while( step < length_of_random_walk && a != b){ // walk when > c or the first step
        a = sample_in_neighbor(a, *g);
        b = sample_in_neighbor(b, *g);
        step ++;
        if(a == -1 || b == -1){
            break;
        }else if (a == b){
            indicator = 1;
            break;
        }
    }
    return c * indicator;
}

size_t residual_set::size() const {
    return hash_d.size();
}

bool residual_set::empty() const {
    return hash_d.empty();
}

void residual_set::clear() {
    sum = 0;
    hash_d.clear();
}

void residual_set::push(NodePair np, double residual) {
    if (hash_d.contains(np)) {
        hash_d[np].residual += residual;
    } else {
        data_item d{np, residual};
        hash_d[np] = d;
    }
    sum += residual;
}

data_item residual_set::pop() { // pop a random element
    auto element_iter = select_randomly(hash_d.begin(), hash_d.end());
    data_item d(element_iter->second); // copy out the data
    hash_d.erase(element_iter);
    sum -= d.residual;
    return d;
}









