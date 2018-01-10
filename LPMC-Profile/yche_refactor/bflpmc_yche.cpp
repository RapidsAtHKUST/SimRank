#include "bflpmc_yche.h"
#include "../util/search_yche.h"

BFLPMC::BFLPMC(string g_name_, GraphYche &g_, double c_, double epsilon_, double delta_) :
        g_name(g_name_), g(&g_), c(c_), epsilon(epsilon_), delta(delta_) {
    // init the two components
    flp = new FLPMC(g_name, *g, c, epsilon, delta, 0); // Q is set to 0
    blp = new BackPush(g_name, *g, c, epsilon / flp->get_rmax(), delta);
}

double BFLPMC::query_one2one(NodePair np) {
    if (np.first == np.second) {
        return 1;
    }

    double blp_p_i = blp->backward_push(np, blp->heap).first;// the estimate value of backward local push
#ifdef DEBUG
    cout << "BLP result: " << blp_p_i << endl;
#endif
    if (blp->heap.empty()) { // the corner case: the residual heap is empty
        return blp_p_i;
    }
    // perform sampling: MC3

    int N = blp->number_of_walkers(blp->heap.sum);
    N = ceil(N / pow(1 - c, 2)); // make up for the bounds for c-walk

#ifdef DEBUG
    cout << "number of samples: " << N << endl;
#endif
    // set up the random number generator
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 generator(rd()); //Standard mersenne_twister_engine seeded with rd()

    // set up the discret distribution
    auto begin = blp->heap.heap.begin();
    auto end = blp->heap.heap.end();
    double r_sum = blp->heap.sum;
    vector<double> weights;
    vector<NodePair> node_pairs;
    for (auto it = begin; it != end; ++it) {
        weights.push_back((*it).residual / blp->heap.sum);
        node_pairs.push_back((*it).np);
    }
//    std::discrete_distribution<int> residuals_dist(weights.begin(), weights.end());
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

    // begin sampling
    double estimate_s_i = 0;
    for (int i = 0; i < N; i++) {
        double sim = 0;
//        int index = residuals_dist(generator); // index for node pairs
//        int index = BinarySearchForGallopingSearch(reinterpret_cast<const double *>(&cdf.front()), 0, cdf.size(),
//                                                   rand_gen.double_rand());
//        int index = GallopingSearch(&cdf.front(), 0, static_cast<uint32_t>(cdf.size()),
//                                    static_cast<int>(rand_gen.double_rand() * YCHE_MAX_INT));
//        int index = GallopingSearchAVX2(&cdf.front(), 0, static_cast<uint32_t>(cdf.size()),
//                                                       static_cast<int>(rand_gen.double_rand() * YCHE_MAX_INT));
        int index = BinarySearchForGallopingSearchAVX2(&cdf.front(), 0, static_cast<uint32_t>(cdf.size()),
                                                       static_cast<int>(rand_gen.double_rand() * YCHE_MAX_INT));

        NodePair sampled_np = node_pairs[index];
        int a, b;
        tie(a, b) = sampled_np;
        // samples from this node pair
        sim += flp->lp->query_P(a, b);
        sim += flp->lp->query_R(a, b);

        double current_estimate = flp->lp->query_P(a, b);
        while (((rand_gen.double_rand() < c)) && (a != b)) {
            a = sample_in_neighbor(a, *g, rand_gen);
            b = sample_in_neighbor(b, *g, rand_gen);

            if (a == -1 || b == -1) {
                break;
            }
            sim += flp->lp->query_R(a, b);
        }
        estimate_s_i += sim / N;
    }
#ifdef DEBUG
    cout << "avg. sim: " << estimate_s_i << endl;
#endif
    return blp_p_i + estimate_s_i * r_sum;
}

BFLPMC::BFLPMC(const BFLPMC &other_obj) {
    rand_gen = SFMTRand();

    g_name = other_obj.g_name;
    epsilon = other_obj.epsilon; // the error bound required by query
    delta = other_obj.delta;
    c = other_obj.c;
    g = other_obj.g; // the pointer to the graph

    flp = new FLPMC(*other_obj.flp);
    blp = new BackPush(g_name, *g, c, epsilon / flp->get_rmax(), delta);
}
