#include "simrank.h"

// ground truth for simrank
TruthSim::TruthSim(string name, DirectedG &g, double c_, double epsilon_) {
    g_name = name;
    n = num_vertices(g);
    c = c_;
    epsilon = epsilon_;
    sim.resize(n, n);
    cout << get_file_path() << endl;
    if (file_exists(get_file_path())) {
        cout << "loading ground truth...." << endl;
        load();
    } else {
        sim.setZero();
        for (size_t i = 0; i < n; i++) {
            sim(i, i) = 1;
        }
    }
    // cout << "simrank matrix size: " << sim.size() << endl;
}

void TruthSim::run(DirectedG &g) {
    // compute the ground truth
    SparseMatrix<float> P(n, n);
    indegree_mat(g, P);
    auto PT = P.transpose();
    // size_t maxIter = size_t(ceil(log(epsilon) / log(c)));
    size_t maxIter = 100; // the groud thruth, iterate enough time
    size_t k = 0;
    cout << "max iterations: " << maxIter << endl;
    while (k < maxIter) {
        sim = PT * sim;
        sim = sim * P;
        sim = c * sim;
        for (size_t i = 0; i < n; i++) {
            sim(i, i) = 1;
        }
        k++;
    }
}


void basic_simrank(DirectedG &g, double c, SimRank_matrix &sim) {
    int K = 50;
    int n = num_vertices(g);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                sim[i][j] = 1;
            } else {
                sim[i][j] = 0;
            }
        }
    }
    for (int k = 0; k < K; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i != j) {
                    double current = 0; // accumulated current simrank
                    int i_indeg = in_degree(i, g);
                    int j_indeg = in_degree(j, g);
                    DirectedG::in_edge_iterator i_in, i_end;
                    tie(i_in, i_end) = in_edges(i, g);
                    for (; i_in != i_end; i_in++) {
                        DirectedG::in_edge_iterator j_in, j_end;
                        tie(j_in, j_end) = in_edges(j, g);
                        for (; j_in != j_end; j_in++) {
                            auto source_i = source(*i_in, g);
                            auto source_j = source(*j_in, g);
                            current += ((c * sim[source_i][source_j]) / (i_indeg * j_indeg));
                        }
                    }
                    sim[i][j] = current;
                }
            }
        }
    }
    // for(int i =0;i<n;i++){
    //     for(int j = 0;j < n;j++){
    //         cout << i << " " << j << ": " << sim[i][j] << endl;
    //     }
    // }
    return;
}
