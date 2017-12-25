#include "bflpmc_yche.h"

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
    double flp_p_i = flp->lp->query_P(np.first, np.second);

    double mc_r_i = 0;

    int total_num_samples = 0;
    // perform sampling: MC3
    auto begin = blp->heap.heap.begin();
    auto end = blp->heap.heap.end();
    double r_sum = blp->heap.sum;
    int N = blp->number_of_walkers(blp->heap.sum);

    double estimate_r_i = 0;
//    cout << format("total number of samples: %s") % N << endl;
    if (N > 0) {
        for (auto it = begin; it != end; ++it) {
            double residual = (*it).residual;
            int n = round(residual * N / r_sum);
            total_num_samples += n;
            for (int i = 0; i < n; ++i) {
                double sum_in_this_round = 0;
                int a, b; // the starting node pair
                tie(a, b) = (*it).np;
                sum_in_this_round += flp->lp->query_R(np.first, np.second);
                while (rand_gen.double_rand() < c && (a != b)) {
                    a = sample_in_neighbor(a, *g, rand_gen);
                    b = sample_in_neighbor(b, *g, rand_gen);
                    if (a == -1 || b == -1) {
                        break;
                    }
                    sum_in_this_round += flp->lp->query_R(a, b);
                }
                estimate_r_i += sum_in_this_round;
            }
        }
        estimate_r_i = estimate_r_i / total_num_samples;
    }

    return blp_p_i + flp_p_i + estimate_r_i;
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
