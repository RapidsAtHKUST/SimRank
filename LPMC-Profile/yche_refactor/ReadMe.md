## Local Push

```cpp
void Reduced_LocalPush::push_to_neighbors(GraphYche &g, NodePair &np, double current_residual) {
    // the push method using reduced linear system
    // out-neighbors of a,b
    auto a = np.first;
    auto b = np.second;
    bool is_singleton = a == b;

    // /* only push to partial pairs*/
    size_t out_degree_i = g.out_degree(a);
    size_t out_degree_j = g.out_degree(b);

//    tie(outi_iter, outi_end) = out_edges(np.first, g);
//    tie(outj_iter, outj_end) = out_edges(np.second, g);// init the iterator

    /*the indicator whether the position is common neighbor */
    vector<bool> outs_i_common(out_degree_i, false);
    vector<bool> outs_j_common(out_degree_j, false);

    if (is_singleton) {
        /* starting push for singleton nodes*/
//        for (; outi_iter != outi_end; outi_iter++) {
//            tie(outj_iter, outj_end) = out_edges(np.second, g);// init the iterator
//            auto out_a = target(*outi_iter, g); // out-neighbor
//            for (; outj_iter != outj_end; outj_iter++) {
//                auto out_b = target(*outj_iter, g);
//                auto indegree_a = in_degree(out_a, g);
//                auto indegree_b = in_degree(out_b, g);
//                auto total_in = indegree_a * indegree_b;
//                if (out_a < out_b) { // only push to partial pairs for a < b
//                    NodePair pab(out_a, out_b); // the node-pair to be pushed to
//                    double inc = c * current_residual / total_in;
//                    push(pab, inc); // do the push action
//                }
//            }
//        }
        for (auto off_out_a = g.off_out[a]; off_out_a < g.off_out[a + 1]; off_out_a++) {
            auto out_nei_a = g.neighbors_out[off_out_a];
            for (auto off_out_b = g.off_out[b]; off_out_b < g.off_out[b + 1]; off_out_b++) {
                auto out_nei_b = g.neighbors_out[off_out_b];
                auto indegree_a = g.in_degree(out_nei_a);
                auto indegree_b = g.in_degree(out_nei_b);
                auto total_in = indegree_a * indegree_b;
                if (out_nei_a < out_nei_b) { // only push to partial pairs for a < b
                    NodePair pab(out_nei_a, out_nei_b); // the node-pair to be pushed to
                    double inc = c * current_residual / total_in;
                    push(pab, inc); // do the push action
                }
            }

        }
    } else {
        /* mark the common neighbors */
//        size_t i, j;
//        for (i = 0; i < out_degree_i; i++) {
//            auto a = target(*(outi_iter + i), g);
//            for (j = 0; j < out_degree_j; j++) {
//                auto b = target(*(outj_iter + j), g);
//                if (a == b) {
//                    outs_i_common[i] = true;
//                    outs_j_common[j] = true;
//                    break;
//                }
//            }
//        }
        const auto off_beg_a = g.off_out[a];
        const auto off_beg_b = g.off_out[b];
        for (auto off_a = off_beg_a; off_a < g.off_out[a+1]; off_a++) {
            auto out_nei_a = g.neighbors_out[off_a];
            for (auto off_b = off_beg_b; off_b < g.off_out[b+1]; off_b++) {
                auto out_nei_b = g.neighbors_out[off_b];
                if (out_nei_a = out_nei_b) {
                    outs_i_common[off_a - off_beg_a] = true;
                    outs_j_common[off_b - off_beg_b] = true;
                    break;
                }
            }
        }

        /* starting push for non-singleton nodes*/
//        auto i_begin_iter = outi_iter; // mark the begining iterator
//        for (; outi_iter != outi_end; outi_iter++) {
//            tie(outj_iter, outj_end) = out_edges(np.second, g);// init the iterator
//            auto j_begin = outj_iter;
//            bool is_i_common = outs_i_common[outi_iter - i_begin_iter];// indicator of whether i is a common neighbor
//            for (; outj_iter != outj_end; outj_iter++) {
//                bool is_j_common = outs_j_common[outj_iter - j_begin];
//
//                auto out_a = target(*outi_iter, g); // out-neighbor
//                auto out_b = target(*outj_iter, g);
//                auto indegree_a = in_degree(out_a, g);
//                auto indegree_b = in_degree(out_b, g);
//                auto total_in = indegree_a * indegree_b;
//                double inc = c * current_residual / total_in;
//                if (out_a == out_b) { //don't push to singleton nodes
//                    continue;
//                }
//                bool oa_less_ob = out_a < out_b ? true : false;
//                // cout << "i com :" << is_i_common << " j com: " << is_j_common << endl;
//                if (!oa_less_ob) {
//                    swap(out_a, out_b);
//                }
//                NodePair pab(out_a, out_b);
//                if (!is_i_common) {
//                    // i is not common neighbonr
//                    push(pab, inc);
//                } else {
//                    // i is a common neighbor
//                    if (is_j_common) {
//                        if (oa_less_ob) {
//                            push(pab, 2 * inc); // push twice for two commons
//                        }
//                    } else {// notmal case
//                        push(pab, inc);
//                    }
//                }
//            }
//        }

        for (auto off_a = off_beg_a; off_a < g.off_out[a+1]; off_a++) {
            auto is_i_common = outs_i_common[off_a - off_beg_a];

            auto out_nei_a = g.neighbors_out[off_a];
            for (auto off_b = off_beg_b; off_b < g.off_out[b+1]; off_b++) {
                auto is_j_common = outs_j_common[off_b - off_beg_b];
                auto out_nei_b = g.neighbors_out[off_b];

                auto indegree_a = g.in_degree(out_nei_a);
                auto indegree_b = g.in_degree(out_nei_b);
                auto total_in = indegree_a * indegree_b;
                double inc = c * current_residual / total_in;
                if (out_nei_a == out_nei_b) { //don't push to singleton nodes
                    continue;
                }
                bool oa_less_ob = out_nei_a < out_nei_b;
                if (!oa_less_ob) { swap(out_nei_a, out_nei_b); }

                NodePair pab(out_nei_a, out_nei_b);
                if (!is_i_common) {
                    // i is not common neighbor
                    push(pab, inc);
                } else {
                    // i is a common neighbor
                    if (is_j_common) {
                        if (oa_less_ob) {
                            push(pab, 2 * inc); // push twice for two commons
                        }
                    } else {// normal case
                        push(pab, inc);
                    }
                }
            }
        }
    }
}
```

```cpp
void Full_LocalPush::push_to_neighbors(GraphYche &g, NodePair &np, double current_residual) {
//    tie(outi_iter, outi_end) = out_edges(np.first, g);
//    for (; outi_iter != outi_end; outi_iter++) {
//        auto out_a = target(*outi_iter, g); // out-neighbor
//        tie(outj_iter, outj_end) = out_edges(np.second, g);
//        for (; outj_iter != outj_end; outj_iter++) {
//            auto out_b = target(*outj_iter, g);
//            if (out_a == out_b) {
//                continue;
//            }
//            auto indegree_a = in_degree(out_a, g);
//            auto indegree_b = in_degree(out_b, g);
//            auto total_in = indegree_a * indegree_b;
//            NodePair pab(out_a, out_b);
//            double inc = c * current_residual / total_in;
//            push(pab, inc);
//        }
//    }

    auto a = np.first;
    auto b = np.second;
    for (auto off_a = g.off_out[a]; off_a < g.off_out[a + 1]; off_a++) {
        auto out_nei_a = g.neighbors_out[off_a];
        for (auto off_b = g.off_out[b]; off_b < g.off_out[b + 1]; off_b++) {
            auto out_nei_b = g.neighbors_out[off_b];
            if (out_nei_a == out_nei_b) {
                continue;
            }
            auto in_degree_a = g.in_degree(out_nei_a);
            auto in_degree_b = g.in_degree(out_nei_b);
            auto total_in = in_degree_a * in_degree_b;
            NodePair pab(out_nei_a, out_nei_b);
            double inc = c * current_residual / total_in;
            push(pab, inc);
        }
    }
}
```

## Prev

modified one

```cpp
pair<double, int> BackPush::backward_push(NodePair np, unique_max_heap &container) 
```

```cpp
auto indeg_a = in_degree(a, *g);
            auto indeg_b = in_degree(b, *g);
            GraphYche::in_edge_iterator ina_it, ina_end;
            GraphYche::in_edge_iterator inb_it, inb_end;
            tie(ina_it, ina_end) = in_edges(a, *g);
            for (; ina_it != ina_end; ++ina_it) {
                auto ina = source(*ina_it, *g);
                tie(inb_it, inb_end) = in_edges(b, *g);
                for (; inb_it != inb_end; ++inb_it) {
                    auto inb = source(*inb_it, *g);
                    // cout << a << " " << b << " pushing to: " << ina << " " << inb << endl;
                    container.push(NodePair{ina, inb}, c * residual / (indeg_a * indeg_b));
                    ++cost;
                }
            }
```

try to use less memory

```cpp
//    GraphYche *g_ptr; // the underlying graph
//    heap_data(NodePair np_, double residual_, GraphYche &g) {
//        np = np_;
//        residual = residual_;
//        g_ptr = &g;
//    }
//

```

update random std

```cpp
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator &g) {
//    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::uniform_int_distribution<> dis(0, (end - start) - 1);
//    std::advance(start, dis(g));
    start += dis(g);
    return start;
}
```