//
// Created by yche on 12/13/17.
//
#include "input_output.h"

#include <cassert>

#include <fstream>
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
            assert(first != second); // no self-loop
            assert(first < INT32_MAX and second < INT32_MAX);
            lines.emplace_back(first, second);
        }
    }
    return lines;
}