#ifndef SLING_H
#define SLING_H

#include <cmath>

#include <utility>
#include <deque>
#include <map>
#include <set>
#include <tuple>

#include <thread>
#include <mutex>

#include <sparsehash/sparse_hash_map>
#include <sparsehash/sparse_hash_set>

#include "SFMT.h"

#include "graph.h"
#include "util.h"

using namespace std;

// template alias
template<typename Key, typename Value, class HashFcn = SPARSEHASH_HASH<Key>>
using gmap = google::sparse_hash_map<Key, Value, HashFcn>;

template<typename Key>
using gset = google::sparse_hash_set<Key>;

class Sling {
// two thread functions
    friend void __Sling_t_calcD(Sling *sim, double eps, mutex *lock, int *cursor, int tid);

    friend void __Sling_t_backward(Sling *sim, double eps, mutex *tasklock, int *cursor, int tid, mutex *plock);

private:
    const static int BLOCKSIZE = 1000;
    static int NUMTHREAD;
//    const static int NUMTHREAD = 2;

    double c;
    double sqrtc;
    double sqrtct[20];
    double d_bar;

    bool *first;
    bool *second;

    Rand gen;
private:
    double calcDi(int i, double eps, bool &early, int &RWCNT, int tid);

    double calcDi_0(int i, double eps, bool &early, int &RWCNT, int tid);

    double calcDi_1(int i, double eps, bool &early, int &RWCNT, int tid);

    double calcDi_2(int i, double eps, bool &early, int &RWCNT, int tid);

    map<pair<int, int>, double, PairCmp> pushback(int u, double eps, int tid);

    vector<double> pushback_q(map<pair<int, int>, double, PairCmp>::iterator start,
                              map<pair<int, int>, double, PairCmp>::iterator end, double eps, int T);

    bool trial(int u, int v, int tid);

    //#################### multi-thread ##############################
    void t_calcD(double eps, mutex *lock, int *cursor, int tid);

    void t_backward(double eps, mutex *tasklock, int *cursor, int tid, mutex *plock);

public:
    Graph *g;

    vector<tuple<int, int, int, double>> p;
    vector<long long> pstart;
    double *d;

    Sling(Graph *g, double c) : g(g), c(c), sqrtc(sqrt(c)), d(NULL), gen(NUMTHREAD) { init(); }

    void init();

    ~Sling() {
        delete[] d;
        delete[] first;
        delete[] second;
    }

    void backward(double eps);

    void calcD(double eps);

    double simrank(int u, int v);

    vector<double> simrank(int u);

    const static double EPS;
    const static double BACKEPS;
    const static double DEPS;
    const static double K;
};

#endif
