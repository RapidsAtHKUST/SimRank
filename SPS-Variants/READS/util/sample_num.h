//
// Created by yche on 1/23/18.
//

#ifndef RELEASE_SAMPLE_NUM_H
#define RELEASE_SAMPLE_NUM_H

#include <cmath>

inline int compute_reads_sample_num(double eps, double delta, double c) {
    return static_cast<int>(ceil(log(delta / 2) * pow(c, 6) / (-2 * eps * eps)));
}

inline int compute_reads_rq_num(double eps, double delta, double c, int r) {
    auto numerator = pow(c, 6);
    auto left_part = -r * pow(eps, 2) / log(delta / 2);
    auto right_part = 2 * eps * pow(c, 3) / 3;
    auto divide_by = 2 * (left_part - right_part);
    return static_cast<int>(ceil(numerator / divide_by));
}

#endif //RELEASE_SAMPLE_NUM_H
