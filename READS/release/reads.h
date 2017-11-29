#ifndef __READS__
#define __READS__

#include <vector>

#include "timer.h"

struct reads {
// data:
    double c;
    int r, n, t;
    char gName[125];
    std::vector<std::vector<int> > nt, ef, eb;
    Timer tm;
    int qCnt;
    double rtime, t1, t2;

// method:
    reads(char *gName_, int n_, int r_, double c_, int t_);

    ~reads();

    double queryOne(int x, int y);

    void queryAll(int x, double *ansVal);

    void queryK(int x, int k, int *ansNode);

};

#endif


