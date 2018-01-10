//
// Created by yche on 1/9/18.
//

#include "search_yche.h"

#include <immintrin.h>  //AVX

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
