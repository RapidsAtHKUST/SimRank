#include "graph_ywangby.h"

string EDGE_LIST_PATH = "./datasets/edge_list/";
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

vector<pair<unsigned int, unsigned int>> GenerateInsEdges(int num_updates, DirectedG &g) {
    size_t n = num_vertices(g);

    vector<pair<unsigned int, unsigned int>> ins_edges;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> distribution(0, static_cast<int>(n - 1));
    while (ins_edges.size() < num_updates) {
        auto e1 = distribution(gen) % n;
        auto e2 = distribution(gen) % n;
        if (!boost::edge(e1, e2, g).second && boost::in_degree(e1, g) > 0 && boost::in_degree(e2, g) > 0) {
            ins_edges.emplace_back(e1, e2);
        }
    }
    return ins_edges;
}

vector<pair<unsigned int, unsigned int>> GenerateDelEdges(int num_updates, DirectedG &g) {
    cout << "begin generate edges..." << endl;
    std::set<std::pair<unsigned int, unsigned int>> del_edges;
    random_device rd;
    mt19937 gen(rd());
    auto num_v = num_vertices(g);

    vector<int> weights;
    weights.reserve(num_v);
    DirectedG::vertex_iterator v_it, v_end;
    tie(v_it, v_end) = vertices(g);
    for (; v_it != v_end; ++v_it) { weights.emplace_back(static_cast<int>(out_degree(*v_it, g))); }

    discrete_distribution<int> geometric_distribution(weights.begin(), weights.end());
    while (del_edges.size() < num_updates) {
        auto src_v = geometric_distribution(gen);
        uniform_int_distribution<int> distribution(0, weights[src_v] - 1);
        DirectedG::out_edge_iterator outi_iter, outi_end;
        tie(outi_iter, outi_end) = out_edges(src_v, g);
        auto dst_v = target(*(outi_iter + distribution(gen)), g);
        del_edges.emplace(src_v, dst_v);
    }

    vector<pair<unsigned int, unsigned int>> del_edge_vec{std::begin(del_edges), std::end(del_edges)};
    return del_edge_vec;
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
