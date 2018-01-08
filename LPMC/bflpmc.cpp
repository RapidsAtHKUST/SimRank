#include "bflpmc.h"

BFLPMC::BFLPMC(string g_name_, DirectedG & g_, double c_, double epsilon_, double delta_):
    g_name(g_name_),
    g(&g_),
    c(c_),
    epsilon(epsilon_),
    delta(delta_){
        // init the two components
        flp = new FLPMC(g_name, *g, c, epsilon, delta, 0); // Q is set to 0
        blp = new BackPush(g_name, *g, c, epsilon / flp->get_rmax(), delta);
    }

double BFLPMC::query_one2one(NodePair np){
    if(np.first == np.second){
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
    
    // set up the random number generator
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 generator(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<double> distribution(0.0,1.0);
    // set up the discret distribution
    vector<double> weights;
    vector<NodePair> node_pairs;
    for(auto it = begin; it !=end; ++ it){
        weights.push_back((*it).residual / blp->heap.sum);
        node_pairs.push_back((*it).np);
    }
    std::discrete_distribution<int> residuals_dist (weights.begin(),weights.end());

    // begin sampling
    double estimate_r_i = 0;
    for(int i = 0 ; i < N ;i ++){
        double current_residual = 0;
        int index =  residuals_dist(generator) ; // index for node pairs
        NodePair sampled_np = node_pairs[index];
        int a,b;
        tie(a,b) = sampled_np;
        // samples from this node pair
        current_residual += flp->lp->query_R(sampled_np.first, sampled_np.second);
        while(distribution(generator) < c && (a !=b )){
            a = sample_in_neighbor(a, *g);
            b = sample_in_neighbor(b, *g);
            if(a == -1 || b == - 1){
                break;
            }
            current_residual += flp->lp->query_R(a, b);
        }
        estimate_r_i += current_residual / N;
    }
    return blp_p_i + estimate_r_i * blp->heap.sum;

    // cout << format("total number of samples: %s") % N << endl;
    // if(N>0){
    //     for(auto it = begin; it !=end; ++it){
    //         double residual = (*it).residual;
    //         int n = round(residual * N / r_sum);
    //         total_num_samples += n;
    //         for(int i = 0 ; i< n;++i){
    //             double sum_in_this_round = 0;
    //             int a,b; // the starting node pair
    //             tie(a,b) = (*it).np;
    //             sum_in_this_round += flp->lp->query_R(np.first, np.second);
    //             while(distribution(gen) < c && (a != b)){
    //                 a = sample_in_neighbor(a, *g);
    //                 b = sample_in_neighbor(b, *g);
    //                 if(a == -1 || b == -1){
    //                     break;
    //                 }
    //                 sum_in_this_round += flp->lp->query_R(a,b);
    //             }
    //             estimate_r_i += sum_in_this_round;
    //         }
    //     }
    //     estimate_r_i = estimate_r_i / total_num_samples;
    // }
    
    // return blp_p_i + flp_p_i + estimate_r_i;
}
