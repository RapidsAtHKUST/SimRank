#ifndef GRAPH_H
#define GRAPH_H

#include <cstdio>
#include <cstdlib>

#include <vector>
#include <algorithm>
#include <iostream>

#include <omp.h>
#include "input_output.h"
#include "util/log.h"

class Graph {
public:
    int n;                          // # of nodes
    size_t m;                          // # of edges
    std::vector<int> *edge;         // list of edges, original graph
    std::vector<int> *inedge;       // list of in-neighbors, reversed graph

    Graph() {
        n = m = 0;
        edge = NULL;
        inedge = NULL;
    }

    ~Graph() {
        delete[] edge;
        delete[] inedge;
    }

    void inputGraph(string &file_path) {
        auto edge_lst = GetEdgeList(file_path);

        n = 0;
        for (auto edge: edge_lst) {
            n = max<int>(n, edge.first);
            n = max<int>(n, edge.second);
        }
        n += 1;
        m = static_cast<size_t>(edge_lst.size());
        log_info("total vertex#: %d", n);
        log_info("total edge#: %d", m);

        edge = new std::vector<int>[n];
        inedge = new std::vector<int>[n];
        for (int i = 0; i < m; ++i) {
            int src, dst;
            std::tie(src, dst) = edge_lst[i];
            edge[src].push_back(dst);
            inedge[dst].push_back(src);
        }

        for (int i = 0; i < n; ++i) {
            sort(edge[i].begin(), edge[i].end());
            sort(inedge[i].begin(), inedge[i].end());
        }
    }

    void inputGraph2(string &file_path) {
        auto edge_lst_size = GetEdgeListBin(file_path);
        auto edge_lst = edge_lst_size.first;
        m = edge_lst_size.second;
        n = 0;
        log_info("Finish Loading");
#pragma omp parallel for reduction(max:n)
        for (size_t i = 0; i < m; ++i) {
            auto edge = edge_lst[i];
            n = max<int>(n, edge.first);
            n = max<int>(n, edge.second);
        }
        n += 1;
        log_info("total vertex#: %d", n);
        log_info("total edge#: %zu", m);

        edge = new std::vector<int>[n];
        inedge = new std::vector<int>[n];
        omp_lock_t *locks = (omp_lock_t *) malloc(sizeof(omp_lock_t) * n);
        for (auto i = 0; i < n; i++) {
            omp_init_lock(&locks[i]);
        }
#pragma omp parallel for
        for (size_t i = 0; i < m; ++i) {
            int src, dst;
            std::tie(src, dst) = edge_lst[i];
            omp_set_lock(&locks[src]);
            edge[src].push_back(dst);
            omp_unset_lock(&locks[src]);

            omp_set_lock(&locks[dst]);
            inedge[dst].push_back(src);
            omp_unset_lock(&locks[dst]);
        }
        for (auto i = 0; i < n; i++) {
            omp_destroy_lock(&locks[i]);
        }
        free(locks);
        log_info("Finish CSR");

#pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            sort(edge[i].begin(), edge[i].end());
            sort(inedge[i].begin(), inedge[i].end());
        }
        log_info("Finish Sort");
        free(edge_lst);
    }
};


#endif
