//
// Created by yche on 1/23/18.
//

#ifndef RELEASE_SAMPLE_NUM_H
#define RELEASE_SAMPLE_NUM_H

#include <cmath>

inline int compute_reads_sample_num(double eps, double delta, double c) {
    return static_cast<int>(ceil(log(delta / 2) * pow(c, 6) / (-2 * eps * eps)));
}

#endif //RELEASE_SAMPLE_NUM_H
