//
// Created by yche on 1/13/18.
//

#include <tuple>
#include <iostream>
#include <boost/format.hpp>

#include "pretty_print.h"

#include "../ground_truth/yche_serialization.h"
#include "../ground_truth/graph_yche.h"

using namespace std;

void SerializeTupleArr() {
    vector<tuple<int, int, int, double>> my_tuple_vec;

    tuple<int, int, int, double> tmp_tuple;
    string my_test_file_path = "./test_tuple_arr.bin";
    if (!file_exists(my_test_file_path)) {
        FILE *pFile = fopen(my_test_file_path.c_str(), "wb");
        YcheSerializer serializer;
        for (auto i = 0; i < 1000; i++) {
            my_tuple_vec.emplace_back(i, i, i + 1, static_cast<double>(i) * i + 1.0 / 3);
            cout << my_tuple_vec[i] << endl;
        }
        serializer.write_tuple_vec(pFile, my_tuple_vec);
        cout << "finished" << endl;
    } else {
        FILE *pFile = fopen(my_test_file_path.c_str(), "r");
        YcheSerializer serializer;
        serializer.read_tuple_vec(pFile, my_tuple_vec);
        cout << my_tuple_vec << endl;
    }
}

void SerializeDoubleArr() {
    vector<double> my_arr;
    for (auto i = 0; i < 100; i++) {
        my_arr.emplace_back(sqrt(i) + 0.1 / 3);
    }
    string my_test_file_path = "./test_double_arr.bin";

    if (!file_exists(my_test_file_path)) {
        FILE *pFile = fopen(my_test_file_path.c_str(), "wb");
        YcheSerializer serializer;
        cout << my_arr << endl;
        serializer.write_array(pFile, &my_arr.front(), my_arr.size());
        cout << "finished" << endl;
    } else {
        FILE *pFile = fopen(my_test_file_path.c_str(), "r");
        YcheSerializer serializer;
        vector<double> arr_b;
        serializer.read_array(pFile, arr_b);
        cout << arr_b << endl;
    }
}

void SerializeBoolArr() {
    auto len = 100;
    auto *my_arr = new bool[len];
    for (auto i = 0; i < len; i++) {
        my_arr[i] = (i % 2 == 0);
    }
    string my_test_file_path = "./test_bool_arr.bin";

    if (!file_exists(my_test_file_path)) {
        FILE *pFile = fopen(my_test_file_path.c_str(), "wb");
        YcheSerializer serializer;
        serializer.write_array(pFile, my_arr, static_cast<size_t>(len));
        cout << "finished" << endl;
    } else {
        FILE *pFile = fopen(my_test_file_path.c_str(), "r");
        YcheSerializer serializer;
        bool *another_arr;
        size_t actual_size;
        serializer.read_array_into_ref(pFile, another_arr, actual_size);
        cout << actual_size << endl;
        for (auto i = 0; i < actual_size; i++) {
            cout << another_arr[i] << endl;
        }
        delete[]another_arr;
    }

    delete[]my_arr;
}

int main(int argc, char *argv[]) {
    double test_num = 0.1;
    cout << boost::format("%s : hello-world %s") % 1 % test_num << endl;
    SerializeTupleArr();
    SerializeDoubleArr();
    SerializeBoolArr();
}