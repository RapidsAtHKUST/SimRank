//
// Created by yche on 7/24/18.
//
#include "Graph.h"

#include <cassert>
#include <sstream>

vector<pair<uint32_t, uint32_t>> GetEdgeList(string &file_path) {
    vector<pair<uint32_t, uint32_t>> lines;

    ifstream ifs(file_path);

    while (ifs.good()) {
        string tmp_str;
        stringstream ss;
        std::getline(ifs, tmp_str);
        if (!ifs.good())
            break;
        if (tmp_str[0] != '#') {
            ss.clear();
            ss << tmp_str;
            int first, second;
            ss >> first >> second;
            assert(first < INT32_MAX and second < INT32_MAX);
//            assert(first != second); // no self-loop
//            if (first != second)
            lines.emplace_back(first, second);
        }
    }
    return lines;
}