#ifndef __BFLPMC_H__
#define __BFLPMC_H__

#include "bprw_yche.h"
#include "flpmc_yche.h"

struct BFLPMC {
    // backward local push and forward local push and MC sampling
public:
    string g_name;
    double epsilon;
    double delta;
    double c;
    GraphYche *g;
    BackPush *blp;
    FLPMC *flp;

    SFMTRand rand_gen;
public:
#ifdef VARYING_RMAX

    BFLPMC(string g_name_, GraphYche &, double c_, double epsilon_, double delta_, double r_max_);

#else

    BFLPMC(string g_name_, GraphYche &, double c_, double epsilon_, double delta_);

#endif

    BFLPMC(const BFLPMC &other_obj);

    ~BFLPMC() {
        delete blp;
        delete flp;
    }

    double query_one2one(NodePair np);
};

#endif
