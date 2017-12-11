/*
 * Author: Shao Yingxia
 * Create Date: 2012骞�2鏈�0鏃�鏄熸湡鍥�19鏃�4鍒�2绉� */
#ifndef __GSINTERFACE_H__
#define __GSINTERFACE_H__

#include "config.h"
#include "simrankvalue.hpp"

class GSInterface {
public:
    GSInterface() {}

    virtual ~GSInterface() {}

    virtual void initialize() {}

    virtual void analysis() {}

    virtual void insertEdge(int sid, int src, int dst) = 0;

    virtual void
    computeSimrank(int sid, vector<SimRankValue> &sim, map<int, vector<pair<int, int> > *> &timestamp, int maxSteps,
                   double df, int qv, int sqn)=0;

    /* dynamic API */
    virtual void update(int src, int dst) {}
};

#endif
