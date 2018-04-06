#ifndef __FILE_SERIALIZATION_H__
#define __FILE_SERIALIZATION_H__

#include <cstdio>
#include <sparsepp/spp.h>

using spp::sparse_hash_map;
using namespace std;

class FileSerializer {
public:
    // serialize basic types to FILE
    // -----------------------------
    template<class T>
    bool operator()(FILE *fp, const T &value) {
        return fwrite((const void *) &value, sizeof(value), 1, fp) == 1;
    }

    template<class T>
    bool operator()(FILE *fp, T *value) {
        return fread((void *) value, sizeof(*value), 1, fp) == 1;
    }

    // serialize std::string to FILE
    // -----------------------------
    bool operator()(FILE *fp, const string &value) {
        const size_t size = value.size();
        return (*this)(fp, size) && fwrite(value.c_str(), size, 1, fp) == 1;
    }

    bool operator()(FILE *fp, string *value) {
        size_t size;
        if (!(*this)(fp, &size))
            return false;
        char *buf = new char[size];
        if (fread(buf, size, 1, fp) != 1) {
            delete[] buf;
            return false;
        }
        new(value) string(buf, (size_t) size);
        delete[] buf;
        return true;
    }

    // serialize std::pair<const A, B> to FILE - needed for maps
    // ---------------------------------------------------------
    template<class A, class B>
    bool operator()(FILE *fp, const std::pair<const A, B> &value) {
        return (*this)(fp, value.first) && (*this)(fp, value.second);
    }

    // 0.001 cut for wiki-Vote, 0.0001 cut for wiki-Link
//    template<class A>
//    bool operator()(FILE *fp, const std::pair<const A, float> &value) {
//        if (value.second < 0.001) { return true; }
//        return (*this)(fp, value.first) && (*this)(fp, value.second);
//    }

    template<class A, class B>
    bool operator()(FILE *fp, std::pair<const A, B> *value) {
        return (*this)(fp, (A *) &value->first) && (*this)(fp, &value->second);
    }

//    template<class A>
//    bool operator()(FILE *fp, std::pair<const A, float> *value) {
//        if (value->second < 0.001) { return true; }
//
//        return (*this)(fp, (A *) &value->first) && (*this)(fp, &value->second);
//    }
};

#endif
