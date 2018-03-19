```cpp

vector<NodePair> e1 = { // the test graph
        {1, 2},
        {2, 4},
        {4, 1},
        {4, 3},
        {3, 1}
};

vector<NodePair> e2 = {
        {6, 8},
        {3, 1},
        {3, 2},
        {3, 5},
        {4, 6},
        {5, 6},
        {3, 4},
        {6, 7}
};
vector<NodePair> e3 = {
        {0, 0},
        {1, 2},
        {3, 1},
        {1, 3},
        {1, 4},
        {2, 1},
        {4, 1}
};
vector<NodePair> e4 = {
        {1, 3},
        {1, 4},
        {1, 5},
        {2, 4},
        {2, 5},
        {2, 6}
};
vector<NodePair> e5 = {
        {0, 1},
        {0, 2},
        {1, 0},
        {1, 2},
        {2, 1},
        {2, 0}
};
vector<NodePair> e6 = {
        {0, 1},
        {0, 2},
        {0, 3},
        {1, 0},
        {1, 2},
        {2, 1},
        {2, 0},
        {3, 0},
        {3, 4},
        {4, 3}
};
vector<NodePair> e7 = {
        {1, 4},
        {2, 1},
        {3, 1},
        {3, 2},
        {4, 3}
};

```

```cpp

void compare_sim(const SimRank_matrix &s1, LocalPush &lp, int n) {
    // compare results with ground truth
    // maximum error
    double error = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            // cout << i << " " << j << ": " << str(format("%.4f") % s1[i][j])
            //     << "    " <<  format("%.4f")% lp.query_P(i,j)
            //     << "    error: " << str(format("%.4f") % (lp.query_P(i,j) - s1[i][j]))
            //             << endl;
            error = max(abs(lp.query_P(i, j) - s1[i][j]), error);
        }
    }
    cout << "maximum error: " << error << endl;
}

```

```cpp

void build_graph(DirectedG &g, vector<NodePair> e) {
    DirectedG::vertex_descriptor i;
    for (auto &item:e) {
        add_edge(item.first, item.second, g);
    }
    // show_graph(g);
}

void display_F(vector<NodePair> &e1) {
    // display the entries in the matrix F
    DirectedG g;
    build_graph(g, e1);
    DirectedG::vertex_iterator v_begin, v_end, v_it, u_it;
    tie(v_begin, v_end) = vertices(g);
    int count = 0;
    for (v_it = v_begin; v_it != v_end; v_it++) {
        for (u_it = v_begin; u_it != v_end; u_it++) {
            int in_v = in_degree((*v_it), g);
            int in_u = in_degree(*u_it, g);
            if (in_v * in_u > 0 && (*v_it != *u_it)) {
                // cout << format("(%s,%s)[%s]: ") % *v_it % *u_it % (in_u * in_v);
                DirectedG::in_edge_iterator v_in_it, v_in_begin, v_in_end;
                DirectedG::in_edge_iterator u_in_it, u_in_begin, u_in_end;
                tie(v_in_begin, v_in_end) = in_edges(*v_it, g);
                tie(u_in_begin, u_in_end) = in_edges(*u_it, g);
                for (v_in_it = v_in_begin; v_in_it != v_in_end; v_in_it++) {
                    for (u_in_it = u_in_begin; u_in_it != u_in_end; u_in_it++) {
                        auto a = source(*v_in_it, g);
                        auto b = source(*u_in_it, g);
                        cout << format("(%s,%s), (%s,%s): %s ") % *v_it % *u_it % a % b % (in_v * in_u) << endl;
                        count++;
                    }
                }
            }
        }
    }
    cout << count << endl;
}

```

```cpp
else {
            // test multi-thread
            // test_reduction();
            // test_localpush("ca-GrQc", c, epsilon, PARALLEL_LOCAL_PUSH_TYPE);
            display_F(e7);
        }
```