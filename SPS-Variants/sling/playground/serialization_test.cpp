//
// Created by yche on 1/13/18.
//

#include <tuple>
#include <iostream>
#include <type_traits>

#include "pretty_print.h"

#include "../ground_truth/yche_serialization.h"
#include "../ground_truth/graph_yche.h"

using namespace std;

void SerializeTupleArr() {
    vector<tuple<int, int, int, double>> my_tuple_vec;

    tuple<int, int, int, double> tmp_tuple;
    string my_test_file_path = "./test.bin";
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
        my_arr.emplace_back(sqrt(i));
    }
    string my_test_file_path = "./test.bin";

    if (!file_exists(my_test_file_path)) {
        FILE *pFile = fopen(my_test_file_path.c_str(), "wb");
        FileSerializer serializer;
        cout << my_arr << endl;
        serializer.write_array(pFile, &my_arr.front(), my_arr.size());
        cout << "finished" << endl;
    } else {
        FILE *pFile = fopen(my_test_file_path.c_str(), "r");
        FileSerializer serializer;
        vector<double> arr_b;
        serializer.read_array(pFile, arr_b);
        cout << arr_b << endl;
    }
}

int main(int argc, char *argv[]) {
//    SerializeTupleArr();
//    bool *my_arr;
//    my_arr = new bool[100];
//    for (auto i = 0; i < 100; i++) { my_arr[i] = static_cast<int>(ceil(sqrt(i))) > 1; }
//
//    string my_test_file_path = "./test.bin";
//    if (!file_exists(my_test_file_path)) {
//        FILE *pFile = fopen(my_test_file_path.c_str(), "wb");
//        FileSerializer serializer;
//        for_each(my_arr, my_arr + 100, [](bool ele) { cout << ele << endl; });
//        serializer.write_array(pFile, my_arr, 100);
//        cout << "finished" << endl;
//    } else {
//        FILE *pFile = fopen(my_test_file_path.c_str(), "r");
//        FileSerializer serializer;
//        bool *arr_b;
//        cout << serializer.read_array(pFile, arr_b) << endl;
//        for_each(arr_b, arr_b + 100, [](bool ele) { cout << ele << endl; });
//        cout << "before delete" << endl;
//        delete[]arr_b;
//    }
//
//    delete[]my_arr;
}
