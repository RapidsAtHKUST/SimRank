//
// Created by yche on 12/21/17.
//

#ifndef SPS_SFMT_BASED_RAND_H
#define SPS_SFMT_BASED_RAND_H

#include <SFMT.h>

class SFMTRand {
private:
    sfmt_t rand_mt;
public:
    SFMTRand() {
        std::srand(std::time(nullptr));
        sfmt_init_gen_rand(&rand_mt, std::rand());
    }

    // [0, 2^32)
    uint32_t uint_rand() { return sfmt_genrand_uint32(&rand_mt); }

    // [0,1)
    double double_rand() { return sfmt_genrand_real2(&rand_mt); }
};

//  randomly choose [start_iter, end_iter), concept requires random access iterator
template<typename Iter>
Iter select_randomly_sfmt(Iter start, Iter end, SFMTRand &sfmt_rand_gen) {
//    auto advance_step = sfmt_rand_gen.uint_rand() % (std::distance(start, end));
    auto advance_step = sfmt_rand_gen.uint_rand() % (end - start);
//    std::advance(start, advance_step);
    // already sure within the range
    start += advance_step;
    return start;
}

#endif //SPS_SFMT_BASED_RAND_H
