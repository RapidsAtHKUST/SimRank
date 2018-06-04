#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <sys/stat.h>

#include <cstdint>
#include <cstdio>
#include <chrono>  // for high_resolution_clock

#include <iostream>
#include <functional>
#include <random>
#include <algorithm>
#include <utility>
#include <fstream>

#include <boost/format.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/multi_array.hpp>

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <sparsepp/spp.h>
#include <google/dense_hash_map>

#include "file_serialization.h"
#include "stat.h"

#define SPP_MIX_HASH
using namespace boost;
using namespace std;
using namespace Eigen;
using spp::sparse_hash_map;
using spp::sparse_hash_set;
using google::dense_hash_map;

// utility function
inline bool file_exists(const std::string &name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

// graph data
extern string EDGE_LIST_PATH;
extern vector<string> DATA_NAME;

// types definition
typedef boost::adjacency_list<vecS, vecS, bidirectionalS> DirectedG;
typedef pair<unsigned int, unsigned int> NodePair;
typedef boost::multi_array<double, 2> SimRank_matrix;

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator &g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

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
            // cout << "hash_value: " << hash_value  << endl;
            return hash_value;
        }
    };

    // string format for Node-pair
    inline ostream &operator<<(std::ostream &os, const NodePair np) {
        // os << np.first << ", " << np.second;
        os << format("(%s,%s)") % np.first % np.second;
        return os;
    }

}
struct eq_node_pair {
    bool operator()(const NodePair &a, const NodePair &b) const {
        return a.first == b.first && a.second == b.second;
    }
};

template<typename T>
struct DensePairMap {
    vector<sparse_hash_map<unsigned int, T, std::hash<unsigned int>>>
            v;

    template<typename D>
    T query(D a, D b) {
        auto &table = v[a];
        if (!table.contains(b)) {
            return 0;
        } else {
            return table[b];
        }
    }

    T &operator[](NodePair np) {
        return v[np.first][np.second];
        // if(np.first <= np.second){
        // }else{
        //     return v[np.second][np.first];
        // }
    }

    void add(size_t n) {
        v.insert(v.end(), n, sparse_hash_map<unsigned int, T, std::hash<unsigned int>>
                {});
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
        // save data to file path
        // ofstream o_file;
        // o_file.open(fp, ios::out);
        // o_file << v.size() << endl; // write the number of nodes
        // for (size_t i = 0; i< v.size();i++){
        //     for(auto &item:v[i]){
        //         o_file << i << " " << item.first <<  " " << item.second << endl; // write the three tuple
        //         // cout << i << item.first << item.second << endl;
        //     }
        // }
        // o_file.close();
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

        // ifstream i_file;
        // i_file.open(fp,ios::in);
        // size_t n;
        // i_file >> n;
        // add(n);
        // unsigned int a,b;
        // T value;
        // while(i_file >> a >> b >> value){
        //     // cout << a << " " << b << " " << value << endl;
        //     v[a][b] = value;
        // }
        // i_file.close();
    }

    void erase(NodePair &np) {
        v[np.first].erase(np.second);
    }
};



// typedef vector<sparse_hash_map<unsigned int, double>> DensePairMap;

// typedef dense_hash_map<NodePair, bool, std::hash<NodePair>, eq_node_pair> DensePairMarker;
// typedef dense_hash_map<NodePair, double, std::hash<NodePair>, eq_node_pair> DensePairMap;
typedef sparse_hash_set<NodePair> PairSet;
typedef sparse_hash_map<NodePair, double> PairHashMap;
typedef sparse_hash_map<NodePair, bool> PairMarker;
typedef sparse_hash_map<int, int> Owg;

// graph function
extern void load_graph(string path, DirectedG &g);

extern void load_reversed_graph(string path, DirectedG &g);

extern void show_graph(DirectedG &g);

extern string get_edge_list_path(string s);

extern void indegree_mat(const DirectedG &g, SparseMatrix<float> &P);// column normalized adjacency matrix

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
} // Eigen::

// sample in-neighbor
extern int sample_in_neighbor(int a, DirectedG &g);

#endif
