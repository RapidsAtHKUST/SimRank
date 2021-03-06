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
//        heap_data hd{node_pair, value};
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

BackPush::BackPush(string g_name_, DirectedG &graph, double c_, double epsilon_, double delta_) :
        heap(graph), g_name(g_name_), g(&graph), c(c_), epsilon(epsilon_), fail_prob(delta_) {
}

size_t BackPush::number_of_walkers(double sum) {
    double r = pow(c * sum / epsilon, 2.0) * log(fail_prob / 2.0) / -2.0;
    return ceil(r);
}

double BackPush::keep_push_cost(unique_max_heap &heap) {
    const heap_data &top_element = heap.top();
    size_t d = in_degree(top_element.np.first, *g) * in_degree(top_element.np.second, *g);
    return d + number_of_walkers(heap.sum - (1 - c) * top_element.residual) / (1 - c);
}

double BackPush::change_to_MC_cost(unique_max_heap &heap) {
    return number_of_walkers(heap.sum) / (1 - c);
}

bool BackPush::is_keep_on_push(unique_max_heap &hp) {
    return keep_push_cost(hp) < change_to_MC_cost(hp);
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
                    // cout << a << " " << b << " pushing to: " << ina << " " << inb << endl;
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

#ifdef DEBUG
    cout << "======= greedy test ======" << endl;
#endif
    double p = backward_push(np, heap).first;
#ifdef DEBUG
    cout << "push estimate: " << p << ", final residuals: " << heap.sum << endl;
#endif
    double mc_estimate = MC_random_walk();

    return p + mc_estimate;
}

double BackPush::MC_random_walk() { // perform random walks based on current residuals in the heap
    // assume that there is no singleton nodes in current residuals
    // int N = ceil( pow(c * heap.sum, 2.0) * log(fail_prob / 2.0)  / (-2 * pow(epsilon, 2)) ); // number of samples
    size_t N = number_of_walkers(heap.sum);
#ifdef DEBUG
    cout << format("require %s pairs of samples") % N << endl;
#endif
    if (N == 0) {
        return 0;
    }

#if !defined(SFMT)
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
#endif

    // sample according to the residual distribution, whether they meet
    auto begin = heap.heap.begin();
    auto end = heap.heap.end();
    double r_sum = heap.sum;
    double mc_estimate = 0;
    int total_num_samples = 0;
    int meeting_count = 0;
    for (auto it = begin; it != end; ++it) {
        // cout << (*it).np << ":" << (*it).residual << endl;
        double residual = (*it).residual;
        double local_sum = 0;
        if (residual > 0) {
            int n = round(residual * N / r_sum);
            // int n = ceil(residual * N / r_sum);
            total_num_samples += n;
            for (int i = 0; i < n; i++) {
#if !defined(SFMT)
                int indicator = sample_one_pair((*it).np, generator, distribution);
#else
                int indicator = sample_one_pair((*it).np);
#endif
                meeting_count += indicator;
            }
            // cout << "starting from " << (*it).np << " " << n << " samples. " << " meeting times " << meeting_count<<  endl; 
        }
    }
    if (total_num_samples > 0) { // we have more than 1 samples
#ifdef DEBUG
        cout << format("total meeting hits %s") % meeting_count << endl;
#endif
        mc_estimate = r_sum * meeting_count / double(total_num_samples);
#ifdef DEBUG
        cout << format("Total samples: %s, MC estimate: %s") % total_num_samples % mc_estimate << endl;
#endif
        return mc_estimate;
    } else { // no samples
        return 0;
    }
}

#if !defined(SFMT)

int BackPush::sample_one_pair(NodePair np, std::default_random_engine &generator,
                              std::uniform_real_distribution<double> &distribution) {
    // sample for one pair of random walk, with (1-c) stop probability
    int a = np.first;
    int b = np.second;
    double prob;
    int indicator = 0;
    // vector<int> path_a{a};
    // vector<int> path_b{b};
    while (true) {
        if (a == b) {
            indicator = 1;
            break;
        }
        // a != b
        prob = distribution(generator);
        // cout << prob << endl;
        if (prob < c) {
            // keep on moving
            a = sample_in_neighbor(a, *g);
            b = sample_in_neighbor(b, *g);
            // path_a.push_back(a);
            // path_b.push_back(b);
            if (a == -1 || b == -1) {
                break;
            }
        } else { // stop
            break;
        }
    }
    // cout << "path a:" ;
    // for(auto & item:path_a){
    //     cout << item << " ";
    // }
    // cout << endl;
    // cout << "path b:" ;
    // for(auto & item:path_b){
    //     cout << item << " ";
    // }
    // cout << endl;
    // cout << "indicator " << indicator << endl;
    return indicator;
}
#else

int BackPush::sample_one_pair(NodePair np) {
    // sample for one pair of random walk, with (1-c) stop probability
    int a = np.first;
    int b = np.second;
    double prob;
    int indicator = 0;
    // vector<int> path_a{a};
    // vector<int> path_b{b};
    while (true) {
        if (a == b) {
            indicator = 1;
            break;
        }
        // a != b
        // cout << prob << endl;
        prob = rand_gen.double_rand();

        if (prob < c) {
            // keep on moving
            a = sample_in_neighbor(a, *g, rand_gen);
            b = sample_in_neighbor(b, *g, rand_gen);
            // path_a.push_back(a);
            // path_b.push_back(b);
            if (a == -1 || b == -1) {
                break;
            }
        } else { // stop
            break;
        }
    }
    // cout << "path a:" ;
    // for(auto & item:path_a){
    //     cout << item << " ";
    // }
    // cout << endl;
    // cout << "path b:" ;
    // for(auto & item:path_b){
    //     cout << item << " ";
    // }
    // cout << endl;
    // cout << "indicator " << indicator << endl;
    return indicator;
}

#endif


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