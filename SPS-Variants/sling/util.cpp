//
// Created by yche on 12/17/17.
//

#include "util.h"

bool cmpTuple(const tuple<int, int, int, double> &t1, const tuple<int, int, int, double> &t2) {
    if (std::get<0>(t1) == std::get<0>(t2)) {
        if (std::get<3>(t1) == std::get<3>(t2)) {
            if (std::get<1>(t1) == std::get<1>(t2)) {
                return std::get<2>(t1) < std::get<2>(t2);
            } else return std::get<1>(t1) > std::get<1>(t2);
        } else return std::get<3>(t1) > std::get<3>(t2);
    } else return std::get<0>(t1) < std::get<0>(t2);
}

bool cmp(const pair<pair<int, int>, double> &p1, const pair<pair<int, int>, double> &p2) {
    if (p1.second == p2.second) {
        if (p1.first.first == p2.first.first) return p1.first.second < p2.first.second;
        else return p1.first > p2.first;
    } else return p1.second > p2.second;
}