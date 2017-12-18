/*
 * Author: Shao Yingxia
 * Create Date: 2012年12月12日 星期三 22时58分41秒
 */

#include "mytime.h"

Time::Time() {}

Time::~Time() {}

void
Time::start() {
    isstart_ = true;
    gettimeofday(&start_, &tz_);
}

void
Time::reset() {
    isstart_ = false;
}

void
Time::stop() {
    gettimeofday(&end_, &tz_);
}

double
Time::getElapsedTime() {
    double res = 0.0;
    double factor = 1e-6;
    if (isstart_) {
        res = (end_.tv_sec - start_.tv_sec) + (end_.tv_usec - start_.tv_usec) * factor;
    }
    return res;
}


