//
// Created by yche on 12/13/17.
//
#include "input_output.h"

#include <cassert>

#include <fstream>
#include <sstream>
#include <util/util.h>
#include <util/timer.h>
#include <malloc.h>
#include <omp.h>

#define PAGE_SIZE (4096)
#define IO_REQ_SIZE (PAGE_SIZE * 32)
#define IO_QUEUE_DEPTH (16)

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

pair<pair<uint32_t, uint32_t> *, size_t> GetEdgeListBin(string &file_path) {
    size_t size = file_size(file_path.c_str());
    size_t num_edges = size / sizeof(uint32_t) / 2;
    vector<pair<uint32_t, uint32_t>> lines;

    using Edge = pair<uint32_t, uint32_t>;
    auto file_fd = open(file_path.c_str(), O_RDONLY | O_DIRECT, S_IRUSR | S_IWUSR);
    Edge *edge_lst = (Edge *) memalign(PAGE_SIZE, size + IO_REQ_SIZE);

    Timer io_timer;
    size_t read_size = 0;
#pragma omp parallel num_threads(IO_QUEUE_DEPTH)
    {
#pragma omp for schedule(dynamic, 1) reduction(+:read_size)
        for (size_t i = 0; i < size; i += IO_REQ_SIZE) {
            auto it_beg = i;
            auto *chars = reinterpret_cast<uint8_t *>(edge_lst);
            auto ret = pread(file_fd, chars + it_beg, IO_REQ_SIZE, it_beg);
            if (ret != IO_REQ_SIZE) {
                log_error("Err, %zu, %zu, %zu, %d", i, it_beg, IO_REQ_SIZE, ret);
            } else {
                read_size += ret;
            }
        }
#pragma omp single
        log_info("%zu, %zu", read_size, size);
    }
    return make_pair(edge_lst, num_edges);
}