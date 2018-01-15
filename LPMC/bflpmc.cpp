#include "bflpmc.h"

BFLPMC::BFLPMC(string g_name_, DirectedG & g_, double c_, double epsilon_, double delta_):
    g_name(g_name_),
    g(&g_),
    c(c_),
    epsilon(epsilon_),
    delta(delta_){
        // init the two components
        flp = new FLPMC(g_name, *g, c, epsilon, delta, 0); // Q is set to 0
        blp = new BackPush(g_name, *g, c, (1-c) * epsilon / flp->get_rmax(), delta);
    }

double BFLPMC::query_one2one(NodePair np){
    if(np.first == np.second){
        return 1;
    }
    
    double blp_p_i = blp->backward_push(np, blp->heap).first;// the estimate value of backward local push
    // cout << "BLP result: " << blp_p_i << endl;

    if(blp->heap.empty()){ // the corner case: the residual heap is empty
        return blp_p_i;
    }

    double h_p_r = 0; // for the results of termination residual model


    // perform sampling: MC3

    int N = blp->number_of_walkers(blp->heap.sum);

    // consider the case directly using \sqrt{c} meeting interpretation BLPMC method now
    int N2 =  ceil(pow( c * blp->heap.sum / epsilon, 2.0) * log(2 / delta) / 2);
    // cout << "r_sum: " << blp->heap.sum << endl;

    // set up the random number generator
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 generator(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<double> distribution(0.0,1.0);
    // set up the discret distribution
    auto begin = blp->heap.heap.begin();
    auto end = blp->heap.heap.end();
    double r_sum = blp->heap.sum;
    vector<double> weights;
    vector<NodePair> node_pairs;

    if(N2 < N){ // using the c-walk meeting model
        // cout << "number of samples N2: " << N2 << endl;
        double estimate_s_i = blp->MC_random_walk(N2);
        return blp_p_i + estimate_s_i;

    }else{ // use the termination residual mode
        cout << "number of samples: " << N << endl;
        
        for(auto it = begin; it !=end; ++ it){
            weights.push_back((*it).residual / blp->heap.sum);
            NodePair current_np = (*it).np;
            node_pairs.push_back(current_np);

            // update hpr component
            h_p_r += (*it).residual * (flp->lp->query_P(current_np.first, current_np.second) + flp->lp->query_R(current_np.first, current_np.second));
        }
        std::discrete_distribution<int> residuals_dist (weights.begin(),weights.end());

        // set up the geometry distribution
        std::geometric_distribution<int> geo_distribution(1-c);

        // begin sampling
        double estimate_r_i = 0;
        for(int i = 0 ; i < N ;i ++){
            double terminate_r = 0;
            int length_of_random_walk = geo_distribution(generator) + 1;
            int step = 0;
            int index =  residuals_dist(generator) ; // index for node pairs
            NodePair sampled_np = node_pairs[index];
            int a,b;
            tie(a,b) = sampled_np;
            // samples from this node pair
            double current_estimate = flp->lp->query_P(a, b);
            while( (step < length_of_random_walk + 1) && (a !=b )){ // random walk length (1 + 1 / (1-c))
                a = sample_in_neighbor(a, *g);
                b = sample_in_neighbor(b, *g);
                step ++;
                if(a == -1 || b == - 1){
                    break;
                }
            }
            if( a != -1 && b != -1){
                terminate_r = flp->lp->query_R(a,b);
            }
            estimate_r_i += terminate_r / N;
        }
        // cout << "avg. terminate r: " << estimate_r_i << endl;
        return blp_p_i + h_p_r + c * r_sum * estimate_r_i / (1-c);
    }



}
