#include "local_push.h"

/* the forward local push and MC sampling algorithm */
struct FLPMC{
    string g_name;
    size_t Q;
    double epsilon; // the error bound required by query
    double delta;
    double c;
    DirectedG * g; // the pointer to the graph 
    LocalPush * lp; // the pointer to the local push index

    FLPMC(string g_name_, DirectedG &, double c_, double epsilon_, double delta_, size_t Q);
    double query_one2one(NodePair np);
    double get_rmax(); // calculate the rmax of local push
    double get_lp_epsilon(); // return the epsilong for lp offline index
    double get_N(); // get the number of samples
};

bool lp_file_exists(string data_name, double c, double epsilon, size_t n, bool is_full = false) {
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
    if (file_exists(data_path)) {
        return true;
    } else {
        return false;
    }

    delete lp_test;
}

