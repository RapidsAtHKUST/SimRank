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

const string SLING_INDEX_DIR = "/export/data/ywangby_datasets/sling16";

class Sling {
// two thread functions
    friend void __Sling_t_calcD(Sling *sim, double eps, mutex *lock, int *cursor, int tid);

    friend void __Sling_t_backward(Sling *sim, double eps, mutex *tasklock, int *cursor, int tid, mutex *plock);

private:
    const static int BLOCKSIZE = 100;
    static int NUMTHREAD;

    double c;
    double sqrtc;
    double sqrtct[20];
    double d_bar;

    bool *first;
    bool *second;

    Rand gen;
public:
    Graph *g;
    string g_name;
    double eps_d;
    double theta;

    vector<tuple<int, int, int, double>> p;
    vector<long long> pstart;
    double *d;

    const static double BACKEPS;
    const static double K;
private:
    double calcDi(int i, double eps, bool &early, int &RWCNT, int tid);

    double calcDi_1(int i, double eps, bool &early, int &RWCNT, int tid);

    map<pair<int, int>, double, PairCmp> pushback(int u, double eps, int tid);

    //#################### multi-thread ##############################
    void t_calcD(double eps, mutex *lock, int *cursor, int tid);

    void t_backward(double eps, mutex *tasklock, int *cursor, int tid, mutex *plock);

    //########## serialization ###########################################
    string get_file_path_base();

    void build_or_load_index();

public:
    double failure_probability = 0.01;

    void init();

    Sling(Graph *g, double c) : g(g), c(c), sqrtc(sqrt(c)), d(NULL), gen(NUMTHREAD) { init(); }

    Sling(Graph *g, string graph_name, double c, double eps_d, double theta) :
            g(g), c(c), sqrtc(sqrt(c)), d(NULL), gen(NUMTHREAD), g_name(graph_name), eps_d(eps_d),
            theta(theta) {
        init();
#ifndef ALWAYS_REBUILD
        build_or_load_index();
#endif
    }

    ~Sling() {
        delete[] d;
        delete[] first;
        delete[] second;
    }

    // indexing 1st step
    void calcD(double eps);

    // indexing 2nd step
    void backward(double eps);

public:
    double simrank(int u, int v);

    vector<double> simrank(int u);
};

#endif
