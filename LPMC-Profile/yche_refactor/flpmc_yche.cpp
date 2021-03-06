#include <cmath>

#include <chrono>

#include <boost/program_options.hpp>

#include "flpmc_yche.h"
#include "simrank.h"
#include "../playground/pretty_print.h"

#include "parallel_local_push/parallel_local_push_yche.h"

using namespace std;
using namespace boost::program_options;

#ifdef VARYING_RMAX
FLPMC::FLPMC(string g_name_, GraphYche &g_, double c_, double epsilon_, double delta_, size_t Q_, double r_max_) :
        g_name(g_name_), g(&g_), c(c_), epsilon(epsilon_), delta(delta_), Q(Q_) {
    // 1st: init r_max
    r_max = r_max_;

    // 2nd: compute or load index
    size_t n = static_cast<size_t>(g->n);
    cout << format("r_max: %s") % get_rmax() << endl;
    lp = new Full_LocalPush(*g, g_name, c, get_lp_epsilon(), n);
    if (!lp_file_exists(g_name, c, get_lp_epsilon(), n, true)) { // test wether the local push index exists
        cout << "local push offline index doesn't exists.. " << endl;
        auto start_time = std::chrono::high_resolution_clock::now();
        lp->local_push(*g);
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;
        cout << format("total indexing cost: %s s") % elapsed.count() << endl; // record the pre-processing time
        cout << format("building compete, saving to %s ") % lp->get_file_path_base() << endl;
        lp->save();
        cout << "saved." << endl;
    } else {
        cout << "offline index exists..loading " << endl;
        lp->load();
    }
}
#else

FLPMC::FLPMC(string g_name_, GraphYche &g_, double c_, double epsilon_, double delta_, size_t Q_) :
        g_name(g_name_), g(&g_), c(c_), epsilon(epsilon_), delta(delta_), Q(Q_) {
    // init local push
    size_t n = static_cast<size_t>(g->n);
    cout << format("r_max: %s") % get_rmax() << endl;
    lp = new Full_LocalPush(*g, g_name, c, get_lp_epsilon(), n);

    {
        // directly compute the index, instead of reading and parsing
        auto clock_start = clock();

        auto start_time = std::chrono::high_resolution_clock::now();

        auto plp = PFLP(*g, g_name, c, get_lp_epsilon(), n);
        plp.local_push(*g);
        swap(lp->P, plp.P);
        swap(lp->R, plp.R);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto clock_end = clock();
        cout << "total indexing cpu time:" << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s"
             << endl;
        std::chrono::duration<double> elapsed = end_time - start_time;
        cout << format("total indexing cost: %s s") % elapsed.count() << endl; // record the pre-processing time
    }

//    if (!lp_file_exists(g_name, c, get_lp_epsilon(), n, true)) { // test wether the local push index exists
//        cout << "local push offline index doesn't exists.. " << endl;
//        auto start_time = std::chrono::high_resolution_clock::now();
//        lp->local_push(*g);
//        auto end_time = std::chrono::high_resolution_clock::now();
//        std::chrono::duration<double> elapsed = end_time - start_time;
//        cout << format("total indexing cost: %s s") % elapsed.count() << endl; // record the pre-processing time
//        cout << format("building compete, saving to %s ") % lp->get_file_path_base() << endl;
//        lp->save();
//        cout << "saved." << endl;
//    } else {
//        cout << "offline index exists..loading " << endl;
//        lp->load();
//    }
}

#endif


FLPMC::FLPMC(const FLPMC &other_obj) {
//    cout << "flpmc..." << endl;
    rand_gen = SFMTRand();

    g_name = other_obj.g_name;
    Q = other_obj.Q;
    epsilon = other_obj.epsilon; // the error bound required by query
    delta = other_obj.delta;
    c = other_obj.c;
    g = other_obj.g; // the pointer to the graph
    lp = other_obj.lp; // the pointer to the local push index

    // rmax
#ifdef VARYING_RMAX
    r_max = other_obj.r_max;
#endif
}

double FLPMC::get_rmax() {
//    double r = sqrt(epsilon);
#ifdef VARYING_RMAX
    return r_max;
#else
    double r = pow(epsilon, 1.0 / 1.5);
    return r;
#endif
//    double r = pow(pow(epsilon, 2) / (pow(c, 3) / log(2.0 / delta)), 1.0 / 3);
//    cout << format("r_max of local push: %s") % r << endl;
}

double FLPMC::get_lp_epsilon() {
//    double lp_epsilon = get_rmax() * (1 - c);
    double lp_epsilon = get_rmax() / (1 - c);
//    cout << format("local push error: %s") % lp_epsilon << endl;
    return lp_epsilon;
}


double FLPMC::query_one2one(NodePair np) {
//    cout << np << endl;
    if (np.first == np.second) {
        return 1;
    } else if (np.first > np.second) { // make sure np.first < np.second
        np = NodePair({np.second, np.first});
    }
    /* FLP result */
    double p_i = lp->query_P(np.first, np.second);
    double r_i = lp->query_R(np.first, np.second);
//    cout << format("pi: %s, ri:%s") % p_i % r_i << endl;

    // set up the random number generator
#ifndef SFMT
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
#endif

    /* MC sampling phase */
    int N = get_N();
    double E_residual = 0;
    for (int i = 0; i < N; i++) {
        int a = np.first;
        int b = np.second;
        int step = 0;
        bool is_dead_node = false;

#ifndef SFMT
        while(( distribution(gen) < c ) && (a != b)){
            a = sample_in_neighbor(a, *g);
            b = sample_in_neighbor(b, *g);
#else
        while ((step == 0 || rand_gen.double_rand() < c) && (a != b)) {
            a = sample_in_neighbor(a, *g, rand_gen);
            b = sample_in_neighbor(b, *g, rand_gen);
#endif
            step++;
            if (a == -1 || b == -1) {
                is_dead_node = true;
                break;
            }
        }
        if (is_dead_node) {
            E_residual += 0;
        } else {
            E_residual += lp->query_R(a, b);
        }
    }

//    cout << format("p_i: %s") % p_i << endl;
//    cout << format("number of samples: %s") % get_N() << endl;
    return p_i + r_i + (c / (1 - c)) * (E_residual / N);
}

double FLPMC::get_N() {
    // compute the number of needed samples 
    double tmp = (pow(c, 2.) * pow(get_rmax(), 2.) * log(2. / delta)) / (2 * pow(epsilon, 2.) * pow((1 - c), 2.));
    int n = ceil(tmp);
    return n;
}
