#ifndef __READSRQ__
#define __READSRQ__

#include <array>

#include "sparsehash/sparse_hash_map"

#include "../util/timer.h"

using namespace std;
using google::sparse_hash_map;

struct readsrq {
public:
    double c;
    int r, n, t, qCnt, rq;
    double rtime, t1, t2;
    Timer tm;

    vector<array<int, 3> > *leaf;
    vector<sparse_hash_map<int, array<int, 3> > > *inode;
/*
leaf[i][x]: i-th simulation, leaf node x
[0]: father(its upper level is level 0)
[1]: left leaf node
[2]: right leaf node

inode[i][j][p]: i-th simulation, level j(starting from 0), node p
.first = p
.second[0]: father
.second[1]: left most leaf node
.second[2]: right most leaf node
*/
    vector<vector<int> > ef, eb;

    char gName[125];
private:
    void loadGraph(char *gName);

    void constructIndices();

    void serializeForSingleSource(Timer &timer, char *iName);

    void deserializeForSingleSource(char *iName);

public:
    readsrq(char *gName_, int n_, int r_, int rq_, double c_, int t_);

    readsrq(string gName_, int n_, int r_, int rq_, double c_, int t_);

    ~readsrq();

    double queryOne(int x, int y, double * ansVal);

    void queryAll(int x, double *ansVal);

    void queryK(int x, int k, int *ansNode);

    void insEdge(int x, int y);

    void delEdge(int x, int y);
};


#endif


