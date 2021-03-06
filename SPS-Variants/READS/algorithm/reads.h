#ifndef __READS__
#define __READS__

#include <vector>
#include <string>

#include "../util/timer.h"

struct reads {
// data:
    double c;
    int r, n, t;
    char gName[125];
    std::vector<std::vector<int>> nt, ef, eb;
    Timer tm;
    int qCnt;
    double rtime, t1, t2;

// methods:
private:
    void loadGraph(char *gName);

    void constructIndices();

    void serializeForSingleSource(Timer &timer, char *iName);

    void deserializeForSingleSource(char *iName);

    void postProcessNextForSinglePair();

public:
    reads(char *gName_, int n_, int r_, double c_, int t_);

    reads(string gName_, int n_, int r_, double c_, int t_);

    ~reads();

    double queryOne(int x, int y);

    double queryOne(int x, int y, double *ansVal);

    void queryAll(int x, double *ansVal);
};

#endif


