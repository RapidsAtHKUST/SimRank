#include "local_push_yche.h"

#include "../util/sfmt_based_rand.h"

/* the forward local push and MC sampling algorithm */
struct FLPMC {
    string g_name;
    size_t Q;
    double epsilon; // the error bound required by query
    double delta;
    double c;

    GraphYche *g; // the pointer to the graph
    LocalPush *lp; // the pointer to the local push index

#ifdef SFMT
    SFMTRand rand_gen;
#endif

#ifdef VARYING_RMAX
    double r_max;
#endif


public:

#ifdef VARYING_RMAX

    FLPMC(string g_name_, GraphYche &, double c_, double epsilon_, double delta_, size_t Q, double r_max_);

#else

    FLPMC(string g_name_, GraphYche &, double c_, double epsilon_, double delta_, size_t Q);

#endif

    FLPMC(const FLPMC &other_obj);

    double query_one2one(NodePair np);

    double get_rmax(); // calculate the rmax of local push
    double get_lp_epsilon(); // return the epsilong for lp offline index
    double get_N(); // get the number of samples
};

inline bool lp_file_exists(string data_name, double c, double epsilon, size_t n, bool is_full = false) {
    LocalPush *lp_test;
    if (is_full) {
        lp_test = new Full_LocalPush();
    } else {
        lp_test = new Reduced_LocalPush();
    }
    // Reduced_LocalPush lp_test;
    lp_test->g_name = data_name;
    lp_test->c = c;
    lp_test->epsilon = epsilon;
    string data_path = lp_test->get_file_path_base() + ".P";
    cout << "data path !!!!" << data_path << endl;
    delete lp_test;

    if (file_exists(data_path)) {
        return true;
    } else {
        return false;
    }

}

