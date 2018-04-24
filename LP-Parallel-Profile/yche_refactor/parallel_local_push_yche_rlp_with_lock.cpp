#ifdef HAS_OPENMP

#include <omp.h>

#endif

#include <algorithm>
#include <fstream>

#include <boost/format.hpp>

#include "parallel_local_push_yche_with_lock.h"

using boost::format;

PRLP::PRLP(GraphYche &g, string name, double c_, double epsilon, size_t n_) : LP(g, name, c_, epsilon, n_) {
    P.add(n);
    R.add(n);
    marker.add(n);

    num_threads = 64u;

    thread_local_expansion_set_lst = vector<vector<int>>(num_threads);
    thread_local_expansion_set_lst[0].reserve(n);
    expansion_pair_lst.resize(n);
    expansion_set_g.resize(n);
    task_hash_table.resize(n);

    for (int i = 0; i < n; i++) {
        NodePair np(i, i);
        R[np] = 1;
        marker[np] = true;
        thread_local_expansion_set_lst[0].emplace_back(i);
        expansion_pair_lst[i].emplace_back(i);
    }

#ifdef HAS_OPENMP
    hash_table_lock = vector<omp_lock_t>(n);
#pragma omp parallel for
    for (auto i = 0; i < hash_table_lock.size(); i++) {
        omp_init_lock(&hash_table_lock[i]);
    }
#endif
}

void PRLP::local_push(GraphYche &g) {
    int counter = 0;
    bool is_go_on;
    auto g_start_time = std::chrono::high_resolution_clock::now();
    auto g_end_time = std::chrono::high_resolution_clock::now();
    auto total_ms = 0;
    long g_expansion_pair_num = 0;
    long g_expansion_a_prime_num = 0;
    long pair_size = 0;
    vector<long> prefix_sum(num_threads + 1, 0);
#pragma omp parallel
    {
#ifdef HAS_OPENMP
        auto thread_id = omp_get_thread_num();
#else
        auto thread_id = 0;
#endif
        auto &local_expansion_set = thread_local_expansion_set_lst[thread_id];

        while (true) {
            // 0th: initialize is_go_on flag
#pragma omp single
            {
                is_go_on = false;
#ifdef DEBUG
                cout << "gen" << endl;
#endif
            }
            if (!local_expansion_set.empty()) { is_go_on = true; }
#pragma omp barrier
            if (!is_go_on) { break; }

#pragma omp single
            {
                // aggregation of v_a for expansion, value in thread_local_expansion_set_lst are distinct
                g_start_time = std::chrono::high_resolution_clock::now();
                g_expansion_pair_num = 0;
                for (auto i = 0; i < thread_local_expansion_set_lst.size(); i++) {
                    prefix_sum[i + 1] = prefix_sum[i] + thread_local_expansion_set_lst[i].size();
                }
                expansion_set_g.resize(static_cast<unsigned long>(prefix_sum.back()));
            }
            std::copy(std::begin(thread_local_expansion_set_lst[thread_id]),
                      std::end(thread_local_expansion_set_lst[thread_id]),
                      std::begin(expansion_set_g) + prefix_sum[thread_id]);

#pragma omp barrier
#pragma omp for
            for (auto i = 0; i < expansion_set_g.size(); i++) {
                g_expansion_pair_num += expansion_pair_lst[expansion_set_g[i]].size();
            }
#ifdef DEBUG
            if (thread_id == 0) {
                cout << "size:" << expansion_set_g.size() << endl;
                cout << "(a, b) to expand:" << g_expansion_pair_num << endl;
            }
#endif

            // 1st: generate tasks
#pragma omp for
            for (auto i = 0; i < task_hash_table.size(); i++) {
                task_hash_table[i].clear();
            }
#pragma omp for schedule(dynamic, 50)
            for (auto i = 0; i < expansion_set_g.size(); i++) {
                auto a = expansion_set_g[i];
                for (auto b:expansion_pair_lst[a]) {

                    NodePair np(a, b);
                    auto &residual_ref = R[np];
                    double residual_to_push = residual_ref;
                    if (a == b) {
                        residual_to_push -= r_max / (1 - c); // singleton nodes do not need to push all residual as 1
                    }

                    marker[np] = false;
                    residual_ref -= residual_to_push;
                    P[np] += residual_to_push;

                    for (auto off_a = g.off_out[a]; off_a < g.off_out[a + 1]; off_a++) {
                        auto out_nei_a = g.neighbors_out[off_a];

                        omp_set_lock(&hash_table_lock[out_nei_a]);
                        task_hash_table[out_nei_a].emplace_back(b, static_cast<float>(residual_to_push), a == b);
                        omp_unset_lock(&hash_table_lock[out_nei_a]);
                    }
                    // important for the later local push-to-neighbors
                    if (a != b) {
                        for (auto off_b = g.off_out[b]; off_b < g.off_out[b + 1]; off_b++) {
                            auto out_nei_b = g.neighbors_out[off_b];

                            omp_set_lock(&hash_table_lock[out_nei_b]);
                            task_hash_table[out_nei_b].emplace_back(a, static_cast<float>(residual_to_push), false);
                            omp_unset_lock(&hash_table_lock[out_nei_b]);
                        }
                    }
                }
            }
#pragma omp single
            {
                pair_size = 0;
                g_expansion_a_prime_num = 0;
            };
#pragma omp for reduction(+:pair_size, g_expansion_a_prime_num)
            for (auto i = 0; i < task_hash_table.size(); i++) {
                pair_size += task_hash_table[i].size();
                g_expansion_a_prime_num += task_hash_table[i].empty() ? 0 : 1;
            }

#pragma omp single
            {
                counter++;
                g_end_time = std::chrono::high_resolution_clock::now();
                auto tmp_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        (g_end_time - g_start_time)).count();
                total_ms += tmp_elapsed;
#ifdef DEBUG
                cout << "(a', b) pair size: " << pair_size << endl;
                cout << "(a', *) size: " << g_expansion_a_prime_num << endl;
                cout << "gen using " << tmp_elapsed << " ms" << endl;
#endif
            }

            // 2nd: task preparation
#pragma omp for nowait
            for (auto i = 0; i < expansion_set_g.size(); i++) { expansion_pair_lst[expansion_set_g[i]].clear(); }
            local_expansion_set.clear();
#pragma omp barrier

            // 3rd: computation
#pragma omp for schedule(dynamic, 1)
            for (auto i = 0; i < task_hash_table.size(); i++) {
                auto out_nei_a = i;
                bool is_enqueue = false;

                for (auto &task :task_hash_table[i]) {
                    // push to neighbors
                    auto local_b = task.b_;
                    if (task.is_singleton_) {
                        // assume neighbors are sorted
                        // only push to partial pairs for a < local_b
                        auto it = std::lower_bound(std::begin(g.neighbors_out) + g.off_out[local_b],
                                                   std::begin(g.neighbors_out) + g.off_out[local_b + 1], out_nei_a);
                        for (auto off_b = it - std::begin(g.neighbors_out) + (*it == out_nei_a ? 1 : 0);
                             off_b < g.off_out[local_b + 1]; off_b++) {
                            auto out_nei_b = g.neighbors_out[off_b];
                            NodePair pab(out_nei_a, out_nei_b);

                            // push
                            auto &res_ref = R[pab];
                            res_ref += sqrt(2) * c * task.residual_ /
                                       (g.in_degree(out_nei_a) * g.in_degree(out_nei_b));

                            if (fabs(res_ref) / sqrt(2) > r_max) { // the criteria for reduced linear system
                                auto &is_in_q_ref = marker[pab];
                                if (!is_in_q_ref) {
                                    expansion_pair_lst[out_nei_a].emplace_back(out_nei_b);
                                    is_enqueue = true;
                                    is_in_q_ref = true;
                                }
                            }
                        }
                    } else {
                        auto it = std::lower_bound(std::begin(g.neighbors_out) + g.off_out[local_b],
                                                   std::begin(g.neighbors_out) + g.off_out[local_b + 1], out_nei_a);
                        for (auto off_b = it - std::begin(g.neighbors_out) + (*it == out_nei_a ? 1 : 0);
                             off_b < g.off_out[local_b + 1]; off_b++) {
                            auto out_nei_b = g.neighbors_out[off_b];
                            NodePair pab(out_nei_a, out_nei_b);

                            // push
                            auto &res_ref = R[pab];
                            res_ref += c * task.residual_ / (g.in_degree(out_nei_a) * g.in_degree(out_nei_b));

                            if (fabs(res_ref) / sqrt(2) > r_max) { // the criteria for reduced linear system
                                auto &is_in_q_ref = marker[pab];
                                if (!is_in_q_ref) {
                                    expansion_pair_lst[out_nei_a].emplace_back(out_nei_b);
                                    is_enqueue = true;
                                    is_in_q_ref = true;
                                }
                            }
                        }
                    }
                }
                if (is_enqueue) { local_expansion_set.emplace_back(out_nei_a); }
            }
        }
#pragma omp for
        for (auto i = 0; i < hash_table_lock.size(); i++) {
            omp_destroy_lock(&hash_table_lock[i]);
        }
    } // end of thread pool

    cout << "rounds:" << counter << "\n gen accumulation:" << total_ms << " ms" << endl;
}

void PRLP::push(NodePair &pab, double inc, bool &is_enqueue) {
    // only probing once
    auto &res_ref = R[pab];
    res_ref += inc;
    if (fabs(res_ref) / sqrt(2) > r_max) { // the criteria for reduced linear system
        auto &is_in_q_ref = marker[pab];
        if (!is_in_q_ref) {
            expansion_pair_lst[pab.first].emplace_back(pab.second);
            is_enqueue = true;
            is_in_q_ref = true;
        }
    }
}

double PRLP::how_much_residual_to_push(GraphYche &g, NodePair &np) {
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

double PRLP::query_P(int a, int b) {
    if (a == b) {
        return P.query(a, b);
    } else if (a > b) {
        return P.query(b, a) / sqrt(2);
    } else {
        return P.query(a, b) / sqrt(2);
    }
}

string PRLP::get_file_path_base() {
    return LOCAL_PUSH_DIR + str(format("RLP_%s-%.3f-%.6f") % g_name % c % epsilon);
}
