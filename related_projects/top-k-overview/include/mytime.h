/*
 * Author: Shao Yingxia
 * Create Date: 2012年12月12日 星期三 22时55分34秒
 */
#ifndef __MYTIME_H__
#define __MYTIME_H__

#include <sys/time.h>

class Time {
public:
    Time();

    ~Time();

    void start();

    void reset();

    void stop();

    double getElapsedTime();

private:
    struct timeval start_;
    struct timeval end_;
    struct timezone tz_;
    bool isstart_;
};

#endif    // #ifndef __TIME_H__

