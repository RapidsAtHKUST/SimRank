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

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 generator; //Standard mersenne_twister_engine seeded with rd()
    std::geometric_distribution<int> geo_distribution;
    SFMTRand rand_gen;
public:
    BFLPMC(string g_name_, GraphYche &, double c_, double epsilon_, double delta_);

    BFLPMC(const BFLPMC &other_obj);

    ~BFLPMC() {
        delete blp;
        delete flp;
    }

    double query_one2one(NodePair np);
};

#endif
