#ifndef __TIMER_H__
#define __TIMER_H__

#include <cstdio>
#include <sys/time.h>

using namespace std;

struct Timer {
    timeval stime, etime;

    Timer() {
        gettimeofday(&stime, NULL);
    }

    void reset() {
        gettimeofday(&stime, NULL);
    }

    double getTime() {
        gettimeofday(&etime, NULL);
        return (double) etime.tv_sec - stime.tv_sec
               + ((double) etime.tv_usec - stime.tv_usec) / 1e6;
    }
};


#endif 