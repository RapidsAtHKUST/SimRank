/*
 * SimMap.hpp
 *
 *  Created on: 2014-10-15
 *      Author: ggbond
 */

#ifndef _SIMMAP_HPP_
#define _SIMMAP_HPP_

#include "../config.h"
#include "../simrankvalue.hpp"

#include "sparsehash/sparse_hash_map"
#include "sparsehash/dense_hash_map"


using std::hash;

/*
struct eqint {
    bool operator()(const int a, const int b) const {
        return a == b;
    }
};
*/

class SimMap {

public:

    SimMap(int root) : master(root) {
        EM = new map<int, double>();
        //	EM = new dense_hash_map<int, double, hash<int>, eqint>;
        //   EM->set_empty_key(-1);
        //    EM->resize(25000000);
    }

    ~SimMap() {}

    // order by desc
    bool operator<(const SimMap &other) const {
        return master > other.master;
    }

    SimMap prioritize(int priority) {
        vector<SimRankValue> CL;
        map<int, double>::iterator it;
        for (it = EM->begin(); it != EM->end(); ++it) {
            CL.push_back(SimRankValue(it->first, it->second));
        }
        sort(CL.begin(), CL.end());
        int size = CL.size() < priority ? CL.size() : priority;
        SimMap res(this->master);
        for (int i = 0; i < size; ++i) {
            res.setValue(CL[i].getVid(), CL[i].getValue());
        }
        clear();
        return res;
    }

    double getValue(int id) {
        if (EM->find(id) == EM->end())
            return 0;
        return (*EM)[id];
    }

    void setValue(int id, double value) {
        (*EM)[id] = value;
    }

    void clear() {
        //EM->clear();
        delete EM;
        EM = NULL;
    }

    int size() {
        return EM->size();
    }

    map<int, double> *EM;
    // google::dense_hash_map<int, double, hash<int>, eqint>* EM;
    int master;
};


#endif /* SIMMAP_HPP_ */
