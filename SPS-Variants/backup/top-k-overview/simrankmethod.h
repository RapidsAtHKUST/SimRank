/*
 * Author: Shao Yingxia
 * Create Date: 2012年12月20日 星期四 19时04分52秒
 */
#ifndef __SIMRANKMETHOD_H__
#define __SIMRANKMETHOD_H__

#include "config.h"
#include "simrankvalue.hpp"

class SimRankMethod {
public:
    SimRankMethod(int maxsteps, int samplenum, double decayfactor) :
            maxSteps(maxsteps), sampleNum(samplenum), decayFactor(decayfactor) { firstRun = true; }

    virtual ~SimRankMethod() {};

    virtual void run(int qv, int k) = 0;

    virtual void update(int src, int dst) {}

    virtual void initialize() {};

    virtual SimRankValue getRes(int idx) {
        return results[idx];
    }

protected:
    int maxSteps;
    int sampleNum;
    double decayFactor;
    vector<SimRankValue> results;
    bool firstRun;//whether first time to call run()

};

#endif
