/*
 * Author: Shao Yingxia
 * Create Date: 2012年12月22日 星期六 21时39分18秒
 */
#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <random>

class Random {
    std::mt19937 generator;
public:
    Random() {
        srand(static_cast<unsigned int>(time(NULL)));
    }

    ~Random() = default;

    unsigned long getRandom() {
        return rand();
    }
};

#endif    // #ifndef __RANDOM_H__

