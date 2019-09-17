#include "bflpmc_yche.h"
#include "../util/search_yche.h"

#ifdef VARYING_RMAX

BFLPMC::BFLPMC(string g_name_, GraphYche &g_, double c_, double epsilon_, double delta_, double r_max_) :
        g_name(g_name_), g(&g_), c(c_), epsilon(epsilon_), delta(delta_) {
    // init the two components
    flp = new FLPMC(g_name, *g, c, epsilon, delta, 0, r_max_); // Q is set to 0
    assert(r_max_ == flp->get_rmax());
    blp = new BackPush(g_name, *g, c, (1 - c) * epsilon / flp->get_rmax(), delta);
}

#else

BFLPMC::BFLPMC(string g_name_, GraphYche &g_, double c_, double epsilon_, double delta_) :
        g_name(g_name_), g(&g_), c(c_), epsilon(epsilon_), delta(delta_) {
    // init the two components
    flp = new FLPMC(g_name, *g, c, epsilon, delta, 0); // Q is set to 0
    blp = new BackPush(g_name, *g, c, (1 - c) * epsilon / flp->get_rmax(), delta);
}

#endif

BFLPMC::BFLPMC(const BFLPMC &other_obj) {
//    cout << "copy constructor...bflpmc" << endl;
    rand_gen = SFMTRand();

    g_name = other_obj.g_name;
    epsilon = other_obj.epsilon; // the error bound required by query
    delta = other_obj.delta;
    c = other_obj.c;
    g = other_obj.g; // the pointer to the graph

    flp = new FLPMC(*(other_obj.flp));
    blp = new BackPush(g_name, *g, c, (1 - c) * epsilon / flp->get_rmax(), delta);
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

    double h_p_r = 0;  // for the results of termination residual model
    // perform sampling: MC3

    int N = blp->number_of_walkers(blp->heap.sum);
    // consider the case directly using \sqrt{c} meeting interpretation BLPMC method now
    int N2 = ceil(pow(c * blp->heap.sum / epsilon, 2.0) * log(2 / delta) / 2);

#ifdef DEBUG
    cout << "number of samples: " << N << endl;
#endif
    if (N2 < N) { // using the c-walk meeting model
        cout << "number of samples N2: " << N2 << endl;

        double estimate_s_i = blp->MC_random_walk(N2);
        return blp_p_i + estimate_s_i;
    } else {
        // discrete distribution
        auto begin = blp->heap.heap.begin();
        auto end = blp->heap.heap.end();
        double r_sum = blp->heap.sum;
        vector<double> weights;
        vector<NodePair> node_pairs;
        for (auto it = begin; it != end; ++it) {
            weights.push_back((*it).residual / blp->heap.sum);
            NodePair current_np = (*it).np;
            node_pairs.push_back(current_np);
            // update hpr component
            h_p_r += (*it).residual * (flp->lp->query_P(current_np.first, current_np.second) +
                                       flp->lp->query_R(current_np.first, current_np.second));
        }
        // init cdf
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
        double estimate_r_i = 0;
        for (int i = 0; i < N; i++) {
            double terminate_r = 0;
            int step = 0;
#ifdef __AVX2__
            int index = BinarySearchForGallopingSearchAVX2(&cdf.front(), 0, static_cast<uint32_t>(cdf.size()),
                                                           static_cast<int>(rand_gen.double_rand() * YCHE_MAX_INT));
#else
            int index = BinarySearchForGallopingSearch(&cdf.front(), 0, static_cast<uint32_t>(cdf.size()),
                    static_cast<int>(rand_gen.double_rand() * YCHE_MAX_INT));
#endif


            NodePair sampled_np = node_pairs[index];
            int a, b;
            tie(a, b) = sampled_np;

            // samples from this node pair
            double current_estimate = flp->lp->query_P(a, b);
            while ((step == 0 || rand_gen.double_rand() < c) && (a != b)) { // random walk length (1 + 1 / (1-c))
                a = sample_in_neighbor(a, *g, rand_gen);
                b = sample_in_neighbor(b, *g, rand_gen);
                step++;
                if (a == -1 || b == -1) {
                    break;
                }
            }
            if (a != -1 && b != -1) {
                terminate_r = flp->lp->query_R(a, b);
            }
            estimate_r_i += terminate_r / N;
        }
        return blp_p_i + h_p_r + c * r_sum * estimate_r_i / (1 - c);
    }
}

