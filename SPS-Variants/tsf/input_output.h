//
// Created by yche on 12/13/17.
//

#ifndef ISP_YCHE_INPUT_OUTPUT_H
#define ISP_YCHE_INPUT_OUTPUT_H

#include <cstdint>

#include <string>
#include <vector>

using namespace std;

vector<pair<uint32_t, uint32_t>> GetEdgeList(string &file_path);

uint32_t InitGraph(string &file_path, vector<int> &off, vector<int> &dst);

#endif //ISP_YCHE_INPUT_OUTPUT_H
