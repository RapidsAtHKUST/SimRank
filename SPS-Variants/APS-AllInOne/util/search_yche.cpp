//
// Created by yche on 1/9/18.
//

#include "search_yche.h"

//#include <immintrin.h>  //AVX
#include <x86intrin.h>

//constexpr int lookup_table[9] = {0x00000000, 0x0000000f, 0x000000ff, 0x00000fff, 0x0000ffff,
//                                 0x000fffff, 0x00ffffff, 0x0fffffff, (int) 0xffffffff};

// the first element satisfying  array[offset] > val
uint32_t BinarySearchForGallopingSearch(const double *array, uint32_t offset_beg, uint32_t offset_end, double val) {
    while (offset_end - offset_beg >= 16) {
        auto mid = (offset_beg + offset_end) / 2;
        _mm_prefetch((char *) &array[(mid + 1 + offset_end) / 2], _MM_HINT_T0);
        _mm_prefetch((char *) &array[(offset_beg + mid) / 2], _MM_HINT_T0);

//  deprecated statement due to double and unlikely
//        if (array[mid] == val) {
//            return mid + 1;             // special case, [xxx,xxx)
//        }
//        else
        if (array[mid] <= val) {
            offset_beg = mid + 1;
        } else {
            offset_end = mid;
        }
    }

    // linear search fallback
    for (auto offset = offset_beg; offset < offset_end; offset++) {
        if (array[offset] > val) {  // special case, [xxx,xxx)
            return offset;
        }
    }
    return offset_end;
}

// 1st: serial galloping search implementaion
uint32_t BinarySearchForGallopingSearch(const int *array, uint32_t offset_beg, uint32_t offset_end, int val) {
    while (offset_end - offset_beg >= 32) {
        auto mid = (offset_beg + offset_end) / 2;
        _mm_prefetch((char *) &array[(mid + 1 + offset_end) / 2], _MM_HINT_T0);
        _mm_prefetch((char *) &array[(offset_beg + mid) / 2], _MM_HINT_T0);
        if (array[mid] == val) {
            return mid;
        } else if (array[mid] < val) {
            offset_beg = mid + 1;
        } else {
            offset_end = mid;
        }
    }

    // linear search fallback
    for (auto offset = offset_beg; offset < offset_end; offset++) {
        if (array[offset] >= val) {
            return offset;
        }
    }
    return offset_end;
}

uint32_t GallopingSearch(int *array, uint32_t offset_beg, uint32_t offset_end, int val) {
//    if (array[offset_end - 1] < val) {
//        return offset_end;
//    }
    // galloping
    if (array[offset_beg] >= val) {
        return offset_beg;
    }
    if (array[offset_beg + 1] >= val) {
        return offset_beg + 1;
    }
    if (array[offset_beg + 2] >= val) {
        return offset_beg + 2;
    }

    auto jump_idx = 4u;
    while (true) {
        auto peek_idx = offset_beg + jump_idx;
        if (peek_idx >= offset_end) {
            return BinarySearchForGallopingSearch(array, (jump_idx >> 1) + offset_beg + 1, offset_end, val);
        }
        if (array[peek_idx] < val) {
            jump_idx <<= 1;
        } else {
            return array[peek_idx] == val ? peek_idx :
                   BinarySearchForGallopingSearch(array, (jump_idx >> 1) + offset_beg + 1, peek_idx + 1, val);
        }
    }
}

#ifdef __AVX2__
// 2nd: avx2-based galloping search
uint32_t BinarySearchForGallopingSearchAVX2(const int *array, uint32_t offset_beg, uint32_t offset_end, int val) {
    while (offset_end - offset_beg >= 16) {
        auto mid = (offset_beg + offset_end) / 2;
        _mm_prefetch((char *) &array[(mid + 1 + offset_end) / 2], _MM_HINT_T0);
        _mm_prefetch((char *) &array[(offset_beg + mid) / 2], _MM_HINT_T0);
        if (array[mid] == val) {
            return mid;
        } else if (array[mid] < val) {
            offset_beg = mid + 1;
        } else {
            offset_end = mid;
        }
    }

    // linear search fallback, be careful with operator>> and operation+ priority
    __m256i pivot_element = _mm256_set1_epi32(val);
    for (; offset_beg + 7 < offset_end; offset_beg += 8) {
        __m256i elements = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(array + offset_beg));
        __m256i cmp_res = _mm256_cmpgt_epi32(pivot_element, elements);
        int mask = _mm256_movemask_epi8(cmp_res);
        if (mask != 0xffffffff) {
            return offset_beg + (_popcnt32(mask) >> 2);
        }
    }
    if (offset_beg < offset_end) {
        auto left_size = offset_end - offset_beg;
        __m256i elements = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(array + offset_beg));
        __m256i cmp_res = _mm256_cmpgt_epi32(pivot_element, elements);
        int mask = _mm256_movemask_epi8(cmp_res);
        int cmp_mask = 0xffffffff >> ((8 - left_size) << 2);
//        int cmp_mask = lookup_table[left_size];
        mask &= cmp_mask;
        if (mask != cmp_mask) { return offset_beg + (_popcnt32(mask) >> 2); }
    }
    return offset_end;
}

uint32_t GallopingSearchAVX2(int *array, uint32_t offset_beg, uint32_t offset_end, int val) {
//    if (array[offset_end - 1] < val) {
//        return offset_end;
//    }

    // linear search
    __m256i pivot_element = _mm256_set1_epi32(val);
    if (offset_end - offset_beg >= 8) {
        __m256i elements = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(array + offset_beg));
        __m256i cmp_res = _mm256_cmpgt_epi32(pivot_element, elements);
        int mask = _mm256_movemask_epi8(cmp_res);
        if (mask != 0xffffffff) { return offset_beg + (_popcnt32(mask) >> 2); }
    } else {
        auto left_size = offset_end - offset_beg;
        __m256i elements = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(array + offset_beg));
        __m256i cmp_res = _mm256_cmpgt_epi32(pivot_element, elements);
        int mask = _mm256_movemask_epi8(cmp_res);
        int cmp_mask = 0xffffffff >> ((8 - left_size) << 2);
        mask &= cmp_mask;
        if (mask != cmp_mask) { return offset_beg + (_popcnt32(mask) >> 2); }
    }

    // galloping, should add pre-fetch later
    auto jump_idx = 8u;
    while (true) {
        auto peek_idx = offset_beg + jump_idx;
        if (peek_idx >= offset_end) {
            return BinarySearchForGallopingSearchAVX2(array, (jump_idx >> 1) + offset_beg + 1, offset_end, val);
        }
        if (array[peek_idx] < val) {
            jump_idx <<= 1;
        } else {
            return array[peek_idx] == val ? peek_idx :
                   BinarySearchForGallopingSearchAVX2(array, (jump_idx >> 1) + offset_beg + 1, peek_idx + 1, val);
        }
    }
}
#endif