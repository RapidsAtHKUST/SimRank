//
// Created by yche on 12/21/17.
//

#ifndef SPS_SFMT_BASED_RAND_H
#define SPS_SFMT_BASED_RAND_H

#include <ctime>

#include <random>

#include <SFMT.h>

class SFMTRand {
private:
    sfmt_t rand_mt;
//    std::uniform_int_distribution<uint32_t> uni_dist;
//    std::uniform_real_distribution<double> uni_dist_double;
//    std::mt19937 gen;
public:
    SFMTRand() {
//        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        sfmt_init_gen_rand(&rand_mt, std::rand());
//        std::random_device rd;
//        gen = std::mt19937(rd());
//        uni_dist = std::uniform_int_distribution<uint32_t>(0, 2 ^ 31);
//        uni_dist_double = std::uniform_real_distribution<double>(0, 1);
    }

    // [0, 2^32)
    uint32_t uint_rand() {
        return sfmt_genrand_uint32(&rand_mt);
//        return uni_dist(gen);
    }

    // [0,1)
    double double_rand() {
        return sfmt_genrand_real2(&rand_mt);
//        return uni_dist_double(gen);
    }
};

//  randomly choose [start_iter, end_iter), concept requires random access iterator
template<typename Iter>
inline Iter select_randomly_sfmt(Iter start, Iter end, SFMTRand &sfmt_rand_gen) {
    auto advance_step = sfmt_rand_gen.uint_rand() % (end - start);
    return start + advance_step;
}

#endif //SPS_SFMT_BASED_RAND_H
