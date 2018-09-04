#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <cstdlib>
#include <cstdio>
#include <ctime>

#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <algorithm>

using namespace std;

vector<pair<uint32_t, uint32_t>> GetEdgeList(string &file_path);

class Graph {
public:
    int n;                          // # of nodes
    int m;                          // # of edges
    std::vector<int> *inAdjList;         // list of edges, original graph
    std::vector<int> *outAdjList;       // list of in-neighbors, reversed graph
    bool is_view;

    Graph(string &file_path) {
        is_view = false;
        n = m = 0;
        inAdjList = NULL;
        outAdjList = NULL;
        inputGraph(file_path);

    }

    Graph(const Graph &graph) {
        is_view = true;
        n = graph.n;
        m = graph.m;
        inAdjList = graph.inAdjList;
        outAdjList = graph.outAdjList;
    }

    ~Graph() {
        if (!is_view) {
            delete[] inAdjList;
            delete[] outAdjList;
        }
    }

    void inputGraph(string &file_path) {
        auto edge_lst = GetEdgeList(file_path);

        n = 0;
        for (auto edge: edge_lst) {
            n = max<int>(n, edge.first);
            n = max<int>(n, edge.second);
        }
        n += 1;
        m = static_cast<int>(edge_lst.size());
        cout << "total vertex#:" << n << endl;
        cout << "total inAdjList#:" << m << endl;

        inAdjList = new std::vector<int>[n];
        outAdjList = new std::vector<int>[n];
        for (int i = 0; i < m; ++i) {
            int src, dst;
            std::tie(src, dst) = edge_lst[i];
            inAdjList[src].push_back(dst);
            outAdjList[dst].push_back(src);
        }

        for (int i = 0; i < n; ++i) {
            sort(inAdjList[i].begin(), inAdjList[i].end());
            sort(outAdjList[i].begin(), outAdjList[i].end());
        }
    }

    int getInSize(int vert) {
        return inAdjList[vert].size();
    }

    int getInVert(int vert, int pos) {
        return inAdjList[vert][pos];
    }

    int getOutSize(int vert) {
        return outAdjList[vert].size();
    }

    int getOutVert(int vert, int pos) {
        return outAdjList[vert][pos];
    }
};

#endif