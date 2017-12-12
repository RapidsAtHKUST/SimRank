#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <cstdlib>
#include <algorithm>
#include <cstdio>
#include <iostream>
#include "graph.h"

class Graph
{
public:
    int n;                          // # of nodes
    int m;                          // # of edges
    std::vector<int> *edge;         // list of edges
    std::vector<int> *inedge;       // list of in-neighbors

    Graph()
    {
        n = m = 0;
        edge = NULL;
        inedge = NULL;
    }
    Graph(DirectedG &g);

    ~Graph()
    {
        delete [] edge;
        delete [] inedge;
    }

    void inputGraph(FILE* f)
    {
        fscanf(f, "%d%d", &n, &m);
        edge = new std::vector<int>[n];
        inedge = new std::vector<int>[n];
        for (int i = 0; i < m; ++i)
        {
//            std::cerr << i << '/' << m << std::endl;
            int x, y;
            fscanf(f, "%d%d", &x, &y);
            edge[x].push_back(y);
            inedge[y].push_back(x);
        }
        for (int i = 0; i < n; ++i)
        {
            sort(edge[i].begin(), edge[i].end());
            sort(inedge[i].begin(), inedge[i].end());
        }
    }

    void outputGraph(FILE* f)
    {
        fprintf(f, "%d %d\n", &n, &m);
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < edge[i].size(); ++j)
            {
                fprintf(f, "%d ", edge[i][j]);
            }
            fprintf(f, "\n");
        }
    }
};

inline std::ostream & operator<<(std::ostream &os, Graph const &sling_g){
    for (int i = 0; i < sling_g.n; ++i){
        os << i << ": ";
        for(auto & k:sling_g.edge[i]){
            os << k << " ";
        }
        os << std::endl;
    }
    return os;
}


#endif
