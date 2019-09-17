//
// Created by yche on 12/17/17.
//

#ifndef SLING_UTIL_H
#define SLING_UTIL_H

#include <SFMT.h>
#include <tuple>

using namespace std;

bool cmpTuple(const tuple<int, int, int, double> &t1, const tuple<int, int, int, double> &t2);

// TODO
bool cmp(const pair<pair<int, int>, double> &p1, const pair<pair<int, int>, double> &p2);

class Rand {
private:
    sfmt_t *sfmt;
public:
    Rand(int NUMTHREAD) {
        sfmt = new sfmt_t[NUMTHREAD];
        for (int i = 0; i < NUMTHREAD; ++i) {
            sfmt_init_gen_rand(sfmt + i, std::rand());
        }
    }

    unsigned rand(int tid) { return sfmt_genrand_uint32(sfmt + tid); }

    double drand(int tid) { return rand(tid) % RAND_MAX / (double) RAND_MAX; }

    ~Rand() {
        delete[] sfmt;
    }
};


class PairCmp {
public:
    bool operator()(const pair<int, int> &p1, const pair<int, int> &p2) {
        if (p1.first == p2.first) return p1.second < p2.second;
        else return p1.first < p2.first;
    }
};

class PairCmp2 {
public:
    bool operator()(const pair<double, int> &p1, const pair<double, int> &p2) {
        if (p1.first == p2.first) return p1.second < p2.second;
        else return p1.first < p2.first;
    }
};

class PairHash {
public:
    size_t operator()(const pair<int, int> &p) const {
        std::hash<int> inthash;
        return inthash(p.first) + inthash(p.second);
    }
};

#endif //SLING_UTIL_H
