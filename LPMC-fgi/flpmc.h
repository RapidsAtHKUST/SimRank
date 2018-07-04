#ifndef __FLPMC_H__
#define __FLPMC_H__

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
    ~FLPMC(){ // the destructor
        delete lp;
        lp = NULL;
        
    }
};

#endif
