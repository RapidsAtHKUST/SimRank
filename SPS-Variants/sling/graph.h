#ifndef GRAPH_H
#define GRAPH_H

#include <cstdio>
#include <cstdlib>

#include <vector>
#include <algorithm>
#include <iostream>

class Graph {
public:
    int n;                          // # of nodes
    int m;                          // # of edges
    std::vector<int> *edge;         // list of edges
    std::vector<int> *inedge;       // list of in-neighbors

    Graph() {
        n = m = 0;
        edge = NULL;
        inedge = NULL;
    }

    ~Graph() {
        delete[] edge;
        delete[] inedge;
    }

    void inputGraph(FILE *f) {
        fscanf(f, "%d%d", &n, &m);
        edge = new std::vector<int>[n];
        inedge = new std::vector<int>[n];
        for (int i = 0; i < m; ++i) {
//            std::cerr << i << '/' << m << std::endl;
            int x, y;
            fscanf(f, "%d%d", &x, &y);
            edge[x].push_back(y);
            inedge[y].push_back(x);
        }
        for (int i = 0; i < n; ++i) {
            sort(edge[i].begin(), edge[i].end());
            sort(inedge[i].begin(), inedge[i].end());
        }
    }

    void outputGraph(FILE *f) {
        fprintf(f, "%d %d\n", &n, &m);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < edge[i].size(); ++j) {
                fprintf(f, "%d ", edge[i][j]);
            }
            fprintf(f, "\n");
        }
    }
};


#endif
