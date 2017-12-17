//
// Created by yche on 12/16/17.
//

#include <iostream>

#include "reads.h"

using namespace std;

// usage and example:
// g++ reads*.h reads*.cpp timer.h -O3 -w -std=c++11 test.cpp -I . && ./a.out hp.data 3133
int main(int argc, char **argv) {
    int n = atoi(argv[2]);
    int r = 1000;
    double c = 0.6;
    int t = 10;
    int rq = 10;

    reads i1(argv[1], n, r, c, t);
    int x = atoi(argv[3]);
    int y = atoi(argv[4]);
    cout << i1.queryOne(x, y) << endl;
}