//
// Created by yche on 3/22/18.
//

#include <fstream>

#include <boost/format.hpp>

#include "local_push_yche.h"

using boost::format;

Reduced_LocalPush::Reduced_LocalPush(GraphYche &g, string name, double c_, double epsilon, size_t n_) :
        LocalPush(g, name, c_, epsilon, n_) {
    string data_path = get_file_path_base() + ".P";
    init_PR();
}

string Reduced_LocalPush::get_file_path_base() {
    return LOCAL_PUSH_DIR + str(format("RLP_%s-%.3f-%.6f") % g_name % c % epsilon);
}

void Reduced_LocalPush::local_push(GraphYche &g) {
    auto start = std::chrono::high_resolution_clock::now();

    while (!Q.empty()) {
        max_q_size = max(max_q_size, Q.size());
        NodePair np = Q.front();
        Q.pop();
//        NodePair np = Q.back();
//        Q.pop_back();

        marker[np] = false;
        auto &R_np_ref = R[np];
        double residual_to_push = R_np_ref;
        if (np.first == np.second) { //singleton node
            residual_to_push -= r_max / (1 - c); // singleton nodes do not need to push all residual as 1
        }

        R_np_ref -= residual_to_push;
        P[np] += residual_to_push;

        // push residuals to neighbors of np
        auto a = np.first;
        auto b = np.second;
        if (a == b) {
            /* starting push for singleton nodes*/
            for (auto off_a = g.off_out[a]; off_a < g.off_out[a + 1]; off_a++) {
                auto out_nei_a = g.neighbors_out[off_a];
                for (auto off_b = g.off_out[b]; off_b < g.off_out[b + 1]; off_b++) {
                    auto out_nei_b = g.neighbors_out[off_b];
                    if (out_nei_a < out_nei_b) { // only push to partial pairs for a < b
                        NodePair pab(out_nei_a, out_nei_b); // the node-pair to be pushed to
                        double inc = c * residual_to_push / (g.in_degree(out_nei_a) * g.in_degree(out_nei_b));
                        n_push++;
                        auto &res_ref = R[pab];
                        res_ref += sqrt(2) * inc;
                        if (fabs(res_ref) / sqrt(2) > r_max) { // the criteria for reduced linear system
                            auto &is_in_q_flag_ref = marker[pab];
                            if (!is_in_q_flag_ref) {
                                Q.push(pab);
//            Q.push_back(pab);
                                is_in_q_flag_ref = true;
                            }
                        }
                    }
                }
            }
        } else {
            /* starting push for non-singleton nodes*/
            for (auto off_a = g.off_out[a]; off_a < g.off_out[a + 1]; off_a++) {
                for (auto off_b = g.off_out[b]; off_b < g.off_out[b + 1]; off_b++) {
                    // put out_nei_a here, since swap happens later
                    auto out_nei_a = g.neighbors_out[off_a];
                    auto out_nei_b = g.neighbors_out[off_b];

                    double inc = c * residual_to_push / (g.in_degree(out_nei_a) * g.in_degree(out_nei_b));
                    if (out_nei_a != out_nei_b) { //don't push to singleton nodes
                        //  a, b is normal in-neighbors of out_a and out_b
                        if (out_nei_a > out_nei_b) {
                            swap(out_nei_a, out_nei_b);
                        }
                        NodePair pab(out_nei_a, out_nei_b);
                        n_push++;
                        auto &res_ref = R[pab];
                        res_ref += inc;
                        if (fabs(res_ref) / sqrt(2) > r_max) { // the criteria for reduced linear system
                            auto &is_in_q_flag_ref = marker[pab];
                            if (!is_in_q_flag_ref) {
                                Q.push(pab);
//            Q.push_back(pab);
                                is_in_q_flag_ref = true;
                            }
                        }
                    }
                }
            }
        }
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    if (cpu_time == -1) {
        cpu_time = elapsed.count();
        mem_size = getValue();
    }
}

void Reduced_LocalPush::push(NodePair &pab, double inc) {
    // only probing once
    n_push++;
    auto &res_ref = R[pab];
    res_ref += inc;
    if (fabs(res_ref) / sqrt(2) > r_max) { // the criteria for reduced linear system
        auto &is_in_q_flag_ref = marker[pab];
        if (!is_in_q_flag_ref) {
            Q.push(pab);
//            Q.push_back(pab);
            is_in_q_flag_ref = true;
        }
    }
}

double Reduced_LocalPush::how_much_residual_to_push(GraphYche &g, NodePair &np) {
    // determine the residual value for current pair to push
    double r = R[np];
    if (np.first == np.second) { //singleton node
        return r - r_max / (1 - c); // singleton nodes do not need to push all residual as 1
    }

#ifdef SELF_LOOP_MERGE
    /* check whether np forms a self-loop */
    if (g.exists_edge(np.first, np.second) && g.exists_edge(np.second, np.first)) { // check whether exists reverse edge
        auto in_deg_a = g.in_degree(np.first);
        auto in_deg_b = g.in_degree(np.second);
        double alpha = c / (in_deg_a * in_deg_b);
        int k = ceil(log(r_max / fabs(r)) / log(alpha));
        double residual_to_push = (1 - pow(alpha, k)) * r / (1 - alpha);
        return residual_to_push;
    }
#endif
    return r;
}

void Reduced_LocalPush::push_to_neighbors(GraphYche &g, NodePair &np, double current_residual) {
    // the push method using reduced linear system
    // out-neighbors of a,b
    auto a = np.first;
    auto b = np.second;
    if (a == b) {
        /* starting push for singleton nodes*/
        for (auto off_a = g.off_out[a]; off_a < g.off_out[a + 1]; off_a++) {
            auto out_nei_a = g.neighbors_out[off_a];
            for (auto off_b = g.off_out[b]; off_b < g.off_out[b + 1]; off_b++) {
                auto out_nei_b = g.neighbors_out[off_b];
                if (out_nei_a < out_nei_b) { // only push to partial pairs for a < b
                    NodePair pab(out_nei_a, out_nei_b); // the node-pair to be pushed to
                    double inc = c * current_residual / (g.in_degree(out_nei_a) * g.in_degree(out_nei_b));
                    push(pab, sqrt(2) * inc);
                }
            }
        }
    } else {
        /* starting push for non-singleton nodes*/
        for (auto off_a = g.off_out[a]; off_a < g.off_out[a + 1]; off_a++) {
            for (auto off_b = g.off_out[b]; off_b < g.off_out[b + 1]; off_b++) {
                // put out_nei_a here, since swap happens later
                auto out_nei_a = g.neighbors_out[off_a];
                auto out_nei_b = g.neighbors_out[off_b];

                double inc = c * current_residual / (g.in_degree(out_nei_a) * g.in_degree(out_nei_b));
                if (out_nei_a != out_nei_b) { //don't push to singleton nodes
                    //  a, b is normal in-neighbors of out_a and out_b
                    if (out_nei_a > out_nei_b) {
                        swap(out_nei_a, out_nei_b);
                    }
                    NodePair pab(out_nei_a, out_nei_b);
                    push(pab, 1 * inc);
                }
            }
        }
    }
}

double Reduced_LocalPush::query_P(int a, int b) {
    if (a == b) {
        return P.query(a, b);
    } else if (a > b) {
        return P.query(b, a) / sqrt(2);
    } else {
        return P.query(a, b) / sqrt(2);
    }
}


