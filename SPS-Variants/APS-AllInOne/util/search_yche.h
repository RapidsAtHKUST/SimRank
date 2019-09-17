//
// Created by yche on 1/9/18.
//

#ifndef LPMC_OPTIMIZING_SEARCH_YCHE_H
#define LPMC_OPTIMIZING_SEARCH_YCHE_H

#include <cstdint>


uint32_t BinarySearchForGallopingSearch(const double *array, uint32_t offset_beg, uint32_t offset_end, double val);

uint32_t BinarySearchForGallopingSearch(const int *array, uint32_t offset_beg, uint32_t offset_end, int val);

uint32_t GallopingSearch(int *array, uint32_t offset_beg, uint32_t offset_end, int val);

#ifdef __AVX2__
// AVX2 implementation
uint32_t BinarySearchForGallopingSearchAVX2(const int *array, uint32_t offset_beg, uint32_t offset_end, int val);

uint32_t GallopingSearchAVX2(int *array, uint32_t offset_beg, uint32_t offset_end, int val);
#endif

#endif //LPMC_OPTIMIZING_SEARCH_YCHE_H
