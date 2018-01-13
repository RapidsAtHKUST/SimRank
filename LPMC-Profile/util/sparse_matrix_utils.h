//
// Created by yche on 12/24/17.
//

#ifndef SPS_SPARSE_MATRIX_UTILS_H
#define SPS_SPARSE_MATRIX_UTILS_H

#include <cstdint>
#include <cstdio>
#include <chrono>  // for high_resolution_clock

#include <algorithm>
#include <iostream>
#include <utility>
#include <fstream>
#include <functional>
#include <random>

#include <boost/format.hpp>

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <sparsepp/spp.h>
#include <sparsehash/dense_hash_map>

#include "../util/file_serialization.h"
#include "../util/stat.h"

using namespace boost;
using namespace std;
using namespace Eigen;

using spp::sparse_hash_map;
using spp::sparse_hash_set;
using google::dense_hash_map;

using NodePair=pair<unsigned int, unsigned int>;

namespace std {
    template<>
    struct hash<NodePair> {
        size_t operator()(NodePair const &p) const { // hash function for a node pair
            unsigned int combined_int;
            // symmetric pairing, we can assume p.second >= p.first
            // combined_int = p.second*(p.second+1)/2 + p.first;

            // elegant pair function
            if (p.first >= p.second) {
                combined_int = p.first * p.first + p.first + p.second;
            } else {
                combined_int = p.second * p.second + p.first;
            }
            auto hash_value = std::hash<unsigned int>{}(combined_int);
            return hash_value;
        }
    };

    // string format for Node-pair
    inline ostream &operator<<(std::ostream &os, const NodePair np) {
        os << format("(%s,%s)") % np.first % np.second;
        return os;
    }
}

template<typename T>
struct DensePairMap {
    vector<sparse_hash_map<unsigned int, T, std::hash<unsigned int>>> v;

    // assume np.first <= np.second
    T &operator[](NodePair np) {
        return v[np.first][np.second];
    }

    template<typename D>
    T query(D a, D b) {
        auto &table = v[a];
        if (!table.contains(b)) {
            return 0;
        } else {
            return table[b];
        }
    }

    void add(size_t n) {
        v.insert(v.end(), n, sparse_hash_map<unsigned int, T, std::hash<unsigned int>>{});
    }

    size_t size() {
        // return the size of 2d matrix
        size_t sum = 0;
        for (auto &h:v) {
            sum += h.size();
        }
        return sum;
    }

    void save(string fp) {
        /* use C-Style I/O binary format*/
        const char *file_name = fp.c_str();
        FILE *o_file = fopen(file_name, "wb");
        if (!o_file) {
            cout << "error opening the file" << endl;
            return;
        }
        auto n = v.size();
        fwrite(&n, sizeof(size_t), 1, o_file); // write vertex number
        FileSerializer fs;
        for (size_t i = 0; i < n; i++) {
            // write the hash map one by one
            v[i].serialize(fs, o_file);
        }
        fclose(o_file);
        return;
    }

    void load(string fp) {
        /* C-style read binary file*/
        const char *file_name = fp.c_str();
        FILE *in_file = fopen(file_name, "rb");
        FileSerializer fs;
        if (!in_file) {
            cout << "error opening the file" << endl;
            return;
        }
        size_t n;
        fread(&n, sizeof(size_t), 1, in_file);
        add(n);
        for (size_t i = 0; i < n; i++) {
            v[i].unserialize(fs, in_file);
        }
        fclose(in_file);
        return;
    }

    void erase(NodePair &np) {
        v[np.first].erase(np.second);
    }
};

// enhance Eigen I/O
namespace Eigen {
    template<class Matrix>
    void write_binary(const char *filename, const Matrix &matrix) {
        std::ofstream out(filename, ios::out | ios::binary | ios::trunc);
        typename Matrix::Index rows = matrix.rows(), cols = matrix.cols();
        out.write((char *) (&rows), sizeof(typename Matrix::Index));
        out.write((char *) (&cols), sizeof(typename Matrix::Index));
        out.write((char *) matrix.data(), rows * cols * sizeof(typename Matrix::Scalar));
        out.close();
    }

    template<class Matrix>
    void read_binary(const char *filename, Matrix &matrix) {
        std::ifstream in(filename, ios::in | std::ios::binary);
        typename Matrix::Index rows = 0, cols = 0;
        in.read((char *) (&rows), sizeof(typename Matrix::Index));
        in.read((char *) (&cols), sizeof(typename Matrix::Index));
        matrix.resize(rows, cols);
        in.read((char *) matrix.data(), rows * cols * sizeof(typename Matrix::Scalar));
        in.close();
    }
}

#endif //SPS_SPARSE_MATRIX_UTILS_H
