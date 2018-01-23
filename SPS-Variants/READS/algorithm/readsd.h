#ifndef __READSD_H__
#define __READSD_H__

#include <array>
#include <vector>
#include "sparsehash/sparse_hash_map"
#include "../util/timer.h"

using namespace std;
using google::sparse_hash_map;

struct readsd {
public:
    double c;
    int r, n, t, qCnt;
    double rtime, t1, t2;
    Timer tm;

    vector<array<int, 3> > *leaf;
    vector<sparse_hash_map<int, array<int, 3> > > *inode;

    vector<vector<int> > ef, eb;

    char gName[125];

public:
    readsd(char *gName_, int n_, int r_, double c_, int t_);

    ~readsd();

    double queryOne(int x, int y);

    void queryAll(int x, double *ansVal);

#ifdef QUERY_K
    void queryK(int x, int k, int *ansNode);
#endif

    void insEdge(int x, int y);

    void delEdge(int x, int y);

};


#endif


