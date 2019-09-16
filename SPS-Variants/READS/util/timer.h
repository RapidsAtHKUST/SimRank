#ifndef __TIMER_H__
#define __TIMER_H__

#include <iostream>
#include <chrono>

using namespace std;

class Timer {
public:
    Timer() : beg_(clock_::now()) {}

    void reset() { beg_ = clock_::now(); }

    double elapsed() const {
        return std::chrono::duration_cast<second_>
                (clock_::now() - beg_).count();
    }

    double getTime() {
        return elapsed();
    }

    double elapsed_and_reset() {
        double elapsed = std::chrono::duration_cast<second_>
                (clock_::now() - beg_).count();
        beg_ = clock_::now();
        return elapsed;
    }

private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> beg_;
};

#endif 