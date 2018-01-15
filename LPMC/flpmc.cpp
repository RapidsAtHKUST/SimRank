#include <boost/program_options.hpp>
#include <cmath>
#include <algorithm> 
#include "flpmc.h"
#include "simrank.h"
using namespace std;
using namespace boost::program_options;

FLPMC::FLPMC(string g_name_, DirectedG &g_, double c_, double epsilon_, double delta_, size_t Q_):
    g_name(g_name_),
    g(&g_),
    c(c_),
    epsilon(epsilon_),
    delta(delta_),
    Q(Q_){
        // init local push
        size_t n = num_vertices(*g);
        cout << format("r_max: %s") % get_rmax() << endl;
        // lp = new Reduced_LocalPush(*g, g_name, c, get_lp_epsilon(), n);
        lp = new Full_LocalPush(*g, g_name, c, get_lp_epsilon(), n);
        if(!lp_file_exists(g_name,c,get_lp_epsilon(),n, true)){ // test wether the local push index exists
            cout << "local push offline index doesn't exists.. " << endl;
            lp->local_push(*g);
            cout << format("building compete, saving to %s " ) % lp->get_file_path_base() << endl;
            lp->save();
            cout << "saved." << endl;
        }else{
            cout << "offline index exists. " << endl;
            lp->load();
        }
    }

double FLPMC::get_rmax(){
    double r = pow(epsilon, 1.0 / 1.5);
    cout << format("r_max of local push: %s") % r << endl;
    return r;
}

double FLPMC::get_lp_epsilon(){
    double lp_epsilon = get_rmax() / (1-c);
    cout << format("local push error: %s") % lp_epsilon << endl;
    return lp_epsilon;
}


double FLPMC::query_one2one(NodePair np){
    if(np.first == np.second){
        return 1 ;
    }else if(np.first > np.second){ // make sure np.first < np.second
        np = NodePair({np.second, np.first});
    }
    /* FLP result */
    double p_i = lp->P[np];
    double r_i = lp->R[np];

    // set up the random number generator
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<double> distribution(0.0,1.0);

    // set up the geometry distribution
    std::geometric_distribution<int> geo_distribution(1-c);

    /* MC sampling phase */
    int N = get_N();
    cout << format("number of samples: %s") % N << endl;
    double E_residual = 0;
    for(int i = 0; i< N;i++){
        // cout << "---------" << endl;
        double sum = 0; // the sum of residuals that passed by
        int a = np.first;
        int b = np.second;
        int length_of_random_walk = geo_distribution(gen);
        int step = 0;
        bool is_dead_node = false;
        while(( step < length_of_random_walk + 1) && (a != b)){
            a = sample_in_neighbor(a, *g);
            b = sample_in_neighbor(b, *g);
            step ++;
            if(a == -1 || b == -1){
                is_dead_node = true;
                break;
            }
        }
        if(is_dead_node){
            E_residual += 0;
        }else{
            E_residual += lp->query_R(a,b);
        }
        // cout << "terminal residual: " <<lp->query_R(a,b) << endl;
        // cout << "---------" << endl;
    }

    cout << format("p_i: %s,r_i:%s, estimate residual: %s ")  % p_i  % r_i %( E_residual / N )<< endl;
    cout << format("number of samples: %s") % get_N() << endl;
    return p_i + r_i + (c / (1-c)) * (E_residual / N);
}

double FLPMC::get_N(){
    // compute the number of needed samples 
    double tmp = ( pow(c,2.) * pow(get_rmax(),2.) * log(2. / delta) ) / ( 2 * pow(epsilon,2.) * pow((1-c),2.));
    int n = ceil(tmp);
    return n;
}


