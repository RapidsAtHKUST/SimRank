#include "sling_graph.h"

Graph::Graph(DirectedG &g) {
    n = num_vertices(g);
    m = num_edges(g);
    edge = NULL;
    inedge = NULL;
    edge = new std::vector<int>[n];
    inedge = new std::vector<int>[n];
    DirectedG::edge_iterator edge_iter, edge_end;
    tie(edge_iter, edge_end) = edges(g);
    for (auto iter = edge_iter; iter != edge_end; iter++) {
        int source_node = source(*iter, g);
        int target_node = target(*iter, g);
        edge[source_node].push_back(target_node);
        inedge[target_node].push_back(source_node);
        for (int i = 0; i < n; ++i) {
            sort(edge[i].begin(), edge[i].end());
            sort(inedge[i].begin(), inedge[i].end());
        }
    }
}
