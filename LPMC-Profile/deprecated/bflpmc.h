#ifndef __BFLPMC_H__
#define __BFLPMC_H__

#include "bprw.h"
#include "flpmc.h"

struct BFLPMC{
    // backward local push and forward local push and MC sampling
    string g_name;
    double epsilon;
    double delta;
    double c;
    DirectedG *g;
    BackPush * blp;
    FLPMC * flp;
    BFLPMC(string g_name_, DirectedG &, double c_, double epsilon_, double delta_);
    ~BFLPMC(){
        delete blp;
        delete flp;
    }
    double query_one2one(NodePair np);
};

#endif
