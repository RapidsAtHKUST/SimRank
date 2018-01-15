//
// Created by yche on 1/13/18.
//

#ifndef SLING_YCHE_SERIALIZATION_H
#define SLING_YCHE_SERIALIZATION_H

#include <cstdio>
#include <tuple>
#include <vector>

using namespace std;

class YcheSerializer {

public:
    // 1st: for sling 4-tuple serialization and deserialization
    template<class A, class B, class C, class D>
    void write_tuple_vec(FILE *fp, const std::vector<std::tuple<A, B, C, D>> &value) {
        size_t size = value.size();
        fwrite(&size, sizeof(size_t), 1, fp);
        for (const auto &my_tuple: value) {
            fwrite((const void *) &my_tuple, sizeof(my_tuple), 1, fp);
        }
    };

    template<class A, class B, class C, class D>
    void read_tuple_vec(FILE *fp, std::vector<std::tuple<A, B, C, D>> &value) {
        size_t size = 0;
        fread(&size, sizeof(size_t), 1, fp);
        value.resize(size);
        for (auto i = 0; i < size; i++) {
            fread((void *) &value[i], sizeof(value[i]), 1, fp);
        }
    };

    // 2nd: write array
    template<class T>
    void write_array(FILE *fp, const T *my_arr, size_t arr_size) {
        fwrite(&arr_size, sizeof(size_t), 1, fp);
        for (auto i = 0; i < arr_size; i++) {
            fwrite(&arr_size, sizeof(T), 1, fp);
        }
    }

    template<class T>
    void read_array(FILE *fp, T *my_arr) {
        size_t arr_size;
        fread(&arr_size, sizeof(size_t), 1, fp);
        my_arr = new T[arr_size];
        for (auto i = 0; i < arr_size; i++) {
            fread((const void *) &my_arr[i], sizeof(my_arr[i]), 1, fp);
        }
    }

    template<class T>
    bool read_array(FILE *fp, vector<T> &my_arr) {
        static_assert(!is_same<T, bool>::value);
        size_t arr_size;
        fread(&arr_size, sizeof(size_t), 1, fp);
        my_arr.resize(arr_size);
        for (auto i = 0; i < arr_size; i++) {
            fread((const void *) &my_arr[i], sizeof(my_arr[i]), 1, fp);
        }
    }
};

#endif //SLING_YCHE_SERIALIZATION_H
