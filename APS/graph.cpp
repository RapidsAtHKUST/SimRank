#include "graph.h"

string EDGE_LIST_PATH = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/";
vector<string> DATA_NAME = {
        "odlis",
        "ca-GrQc",
        "p2p-Gnutella06",
        "ca-HepTh",
        "wiki-Vote",
        "web-NotreDame",
        "web-Stanford",
        "web-BerkStan",
        "web-Google",
        "soc-pokec-relationships",
        "soc-LiveJournal1"};
double C = 0.6;

void load_graph(string path, DirectedG &g) {
    // load graph from edge_list file
    if (file_exists(path)) {
        int a, b;
        cout << "loading " << path << endl;;
        ifstream edgeFile(path, ios::in);
        while (edgeFile >> a >> b) {
            add_edge(a, b, g);
        }
        edgeFile.close();
        return;
    } else {
        cout << "file doesn't exit" << endl;
        return;
    }
}

void load_reversed_graph(string path, DirectedG &g) {
    // load graph from edge_list file
    if (file_exists(path)) {
        int a, b;
        cout << "loading " << path << endl;;
        ifstream edgeFile(path, ios::in);
        while (edgeFile >> a >> b) {
            add_edge(b, a, g);
        }
        edgeFile.close();
        return;
    } else {
        cout << "file doesn't exit" << endl;
        return;
    }
}

void show_graph(DirectedG &g) {
    DirectedG::vertex_iterator v_it, v_end;
    DirectedG::out_edge_iterator e_it, e_end;
    tie(v_it, v_end) = vertices(g);
    cout << "Adj lists: " << endl;
    for (; v_it != v_end; v_it++) {
        cout << *v_it << ": ";
        tie(e_it, e_end) = out_edges(*v_it, g);
        for (; e_it != e_end; e_it++) {
            cout << target(*e_it, g) << " ";
        }
        cout << endl;
    }
}

string get_edge_list_path(string s) {
    // get file location of edgelist for graph s
    return EDGE_LIST_PATH + s + ".txt";
}

void indegree_mat(const DirectedG &g, SparseMatrix<float> &P) {
    // column normalized adjacency matrix 
    typedef Triplet<float> T;
    vector<T> tripleList;
    DirectedG::edge_iterator edge_iter, edge_end;
    tie(edge_iter, edge_end) = edges(g);
    for (; edge_iter != edge_end; edge_iter++) {
        auto a = source(*edge_iter, g);
        auto b = target(*edge_iter, g);
        tripleList.push_back(T(a, b, 1));
    }
    P.setFromTriplets(tripleList.begin(), tripleList.end());
    for (int k = 0; k < P.outerSize(); ++k) { // column normalize the matrix
        auto indeg_k = in_degree(k, g);
        for (SparseMatrix<float>::InnerIterator it(P, k); it; ++it) {
            it.valueRef() = 1.0 / indeg_k;
        }
    }
}

int sample_in_neighbor(int a, DirectedG &g) {
    // sample one in-neighbor of node a in g
    auto in_deg = in_degree(a, g);
    if (in_deg > 0) {
        DirectedG::in_edge_iterator in_edge_iter, in_edge_end;
        tie(in_edge_iter, in_edge_end) = in_edges(a, g);
        return source(*select_randomly(in_edge_iter, in_edge_end), g);
    } else {
        return -1;
    }
}
