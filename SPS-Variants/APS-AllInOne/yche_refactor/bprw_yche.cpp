#include "../util/search_yche.h"
#include "bprw_yche.h"

#include <iostream>

GraphYche *g_ptr = nullptr;

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
#ifdef SPARSE_HASH_MAP_FOR_HEAP
    if (R.contains(node_pair)) {
        // the key is already exists, so udpate its value
        handle_t handle = R[node_pair];
        // udpate sum
        (*handle).residual += value;
        heap.increase(handle);
    } else {
//        heap_data hd{node_pair, value, *g_ptr};
        heap_data hd{node_pair, value};
        handle_t t = heap.push(hd);
        R[node_pair] = t;  // insert the handler to the hash map
    }
    sum += value;
#else
    auto iter = R.find(node_pair);
    if (iter != R.end()) {
        // the key is already exists, so update its value
        handle_t handle = iter->second;
        // update sum
        (*handle).residual += value;
        heap.increase(handle);
    } else {
//        heap_data hd{node_pair, value, *g_ptr};
        heap_data hd{node_pair, value};
        handle_t t = heap.push(hd);
        R.emplace(node_pair, t);// insert the handler to the hash map
    }
    sum += value;
#endif
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

BackPush::BackPush(string g_name_, GraphYche &graph, double c_, double epsilon_, double delta_) :
        heap(graph), g_name(g_name_), g(&graph), c(c_), epsilon(epsilon_), fail_prob(delta_) {
    g_ptr = &graph;
}

size_t BackPush::number_of_walkers(double sum) {
    double r = pow(c * sum / epsilon, 2.0) * log(fail_prob / 2.0) / -2.0;
    return ceil(r);
}

double BackPush::keep_push_cost(unique_max_heap &heap) {
    const heap_data &top_element = heap.top();
    // please use size_t, otherwise probably overflow
    size_t d;
    d = g->in_deg_arr[top_element.np.first] * g->in_deg_arr[top_element.np.second];
    return push_cost * d +
           mc_cost * number_of_walkers(heap.sum - (1 - c) * top_element.residual) * (1 + 1 / (1 - c));
}

double BackPush::change_to_MC_cost(unique_max_heap &heap) {
    return mc_cost * number_of_walkers(heap.sum) * (1.0 + 1.0 / (1 - c));
}

bool BackPush::is_keep_on_push(unique_max_heap &hp) {
    const heap_data &top_element = heap.top();

    return (top_element.np.first == top_element.np.second) ||
           (keep_push_cost(hp) < change_to_MC_cost(hp)); // when singleton nodes, directly keep on push
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
#ifdef DEBUG
        cout << top_element << endl;
#endif
        auto a = top_element.np.first;
        auto b = top_element.np.second;
        double residual = top_element.residual;
        // cout << top_element << " sum: " << container.sum << endl;
        if (a == b) { // current_pair is singleton
            p += residual;
            continue;
        } else { // non-singleton nodes
            auto indeg_a = g->in_deg_arr[a];
            auto indeg_b = g->in_deg_arr[b];
            auto off_a_beg = g->off_in[a];
            auto off_a_end = g->off_in[a + 1];
            auto off_b_beg = g->off_in[b];
            auto off_b_end = g->off_in[b + 1];

            for (auto off_a = off_a_beg; off_a < off_a_end; off_a++) {
                auto in_nei_a = g->neighbors_in[off_a];
                for (auto off_b = off_b_beg; off_b < off_b_end; off_b++) {
                    auto in_nei_b = g->neighbors_in[off_b];
#ifdef DEBUG
                    cout << a << " " << b << " pushing to: " << in_nei_a << " " << in_nei_b << " ," << indeg_a << " ," << indeg_b
                         << endl;
#endif
                    container.push(NodePair{in_nei_a, in_nei_b}, c * residual / (indeg_a * indeg_b));
                    ++cost;
                }
            }
        }
    }
    // cout << format("Total number of pushes: %s") % cost << endl;
    // cout << format{"deterministic estimate: %s, final residual sum: %s"} % p  % container.sum << endl;
    return {p, cost};
}

double BackPush::query_one2one(NodePair np) { // pairwise SimRank estimation
    if (np.first == np.second) { return 1.0; }
#ifdef DEBUG
    cout << "======= greedy test ======" << endl;
#endif
    double p = backward_push(np, heap).first;
#ifdef DEBUG
    cout << "push estimate: " << p << ", final residuals: " << heap.sum << endl;
#endif
    double mc_estimate = MC_random_walk(number_of_walkers(heap.sum));

    return p + mc_estimate;
}

double BackPush::MC_random_walk(int N) { // perform random walks based on current residuals in the heap
    // assume that there is no singleton nodes in current residuals
    // int N = ceil( pow(c * heap.sum, 2.0) * log(fail_prob / 2.0)  / (-2 * pow(epsilon, 2)) ); // number of samples
    if (heap.empty()) { // corner case: heap is empty
        return 0;
    }
//    size_t N = number_of_walkers(heap.sum);
#ifdef DEBUG
    cout << format("require %s pairs of samples") % N << endl;
#endif
    if (N == 0) {
        return 0;
    }

    double mc_estimate = 0;

    // set up the discrete distribution
    auto seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
    std::default_random_engine generator(seed);

#if !defined(SFMT)
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
#endif

    vector<double> weights;
    vector<NodePair> node_pairs;
    auto begin = heap.heap.begin();
    auto end = heap.heap.end();
    for (auto it = begin; it != end; ++it) {
        weights.push_back((*it).residual / heap.sum);
        node_pairs.push_back((*it).np);
    }

#if !defined(SFMT)
    std::discrete_distribution<int> residuals_dist(weights.begin(), weights.end());
#else
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

//    vector<double> cdf(weights.size(), 0.0);
//    auto prev = 0.0;
//    for (auto i = 0; i < weights.size(); i++) {
//        cdf[i] = prev + weights[i];
//        prev = cdf[i];
//    }
#endif

    // begin sampling
    double meeting_count = 0;
    for (int i = 0; i < N; i++) {
#if !defined(SFMT)
        int index = residuals_dist(generator); // index for node pairs
#else

//        int index = GallopingSearch(&cdf.front(), 0, static_cast<uint32_t>(cdf.size()),
//                                    static_cast<int>(rand_gen.double_rand() * YCHE_MAX_INT));
//        int index = GallopingSearchAVX2(&cdf.front(), 0, static_cast<uint32_t>(cdf.size()),
//                                                       static_cast<int>(rand_gen.double_rand() * YCHE_MAX_INT));

#ifdef __AVX2__
        int index = BinarySearchForGallopingSearchAVX2(&cdf.front(), 0, static_cast<uint32_t>(cdf.size()),
                                                       static_cast<int>(rand_gen.double_rand() * YCHE_MAX_INT));
#else
        int index = BinarySearchForGallopingSearch(reinterpret_cast<const double *>(&cdf.front()), 0, cdf.size(),
                                                   rand_gen.double_rand());
#endif

#endif
        NodePair sampled_np = node_pairs[index];

        if (sampled_np.first == sampled_np.second) {
            meeting_count += 1.0;
        } else {
#if !defined(SFMT)
            double sample_result = sample_one_pair(sampled_np, generator, distribution);
#else
            double sample_result = sample_one_pair(sampled_np);
#endif
            meeting_count += sample_result;
        }
    }

    mc_estimate += heap.sum * (meeting_count / double(N));
    return mc_estimate;
}

#if !defined(SFMT)

double BackPush::sample_one_pair(NodePair np, std::default_random_engine &generator,
                             std::uniform_real_distribution<double> &distribution) {
// sample for one pair of random walk, with expected (1 + 1 / (1-c))
// assume that a != b
int a = np.first;
int b = np.second;
double indicator = 0;
int step = 0; //
while ((distribution(generator) < c || step == 0) && a != b) { // walk when > c or the first step
    a = sample_in_neighbor(a, *g);
    b = sample_in_neighbor(b, *g);
    step++;
    if (a == -1 || b == -1) {
        break;
    } else if (a == b) {
        indicator = 1;
        break;
    }
}
return c * indicator;
}
#else

double BackPush::sample_one_pair(NodePair np) {
    // sample for one pair of random walk, with (1-c) stop probability
    // sample for one pair of random walk, with expected (1 + 1 / (1-c))
    // assume that a != b
    int a = np.first;
    int b = np.second;
    double indicator = 0;
    int step = 0; //
    while ((step == 0 || rand_gen.double_rand() < c) && a != b) { // walk when > c or the first step
        a = sample_in_neighbor(a, *g, rand_gen);
        b = sample_in_neighbor(b, *g, rand_gen);
        step++;
        if (a == -1 || b == -1) {
            break;
        } else if (a == b) {
            indicator = 1;
            break;
        }
    }
    return c * indicator;
}

#endif
