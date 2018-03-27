#ifdef HAS_OPENMP

#include <omp.h>

#endif

#include <fstream>

#include <boost/format.hpp>

#include "parallel_local_push_yche.h"

using boost::format;

double cal_rmax(double c, double epsilon) {
    return (1 - c) * epsilon;
}

LP::LP(GraphYche &g, string gName_, double c_, double epsilon_, size_t n_) :
        g_name(gName_), c(c_), epsilon(epsilon_), n(n_) {
    r_max = cal_rmax(c, epsilon);
}

PFLP::PFLP(GraphYche &g, string name, double c_, double epsilon, size_t n_) : LP(g, name, c_, epsilon, n_) {
    P.add(n);
    R.add(n);
    marker.add(n);

    num_threads = 56u;
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

void PFLP::local_push(GraphYche &g) {
    int counter = 0;
    bool is_go_on;

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
                cout << "gen" << endl;
            }
            if (!local_expansion_set.empty()) { is_go_on = true; }
#pragma omp barrier
            if (!is_go_on) {
                break;
            }

#pragma omp single
            {
                // aggregation of v_a for expansion
                std::unordered_set<int> my_set;
                for (auto &expansion_set: thread_local_expansion_set_lst) {
                    for (auto u:expansion_set) {
                        my_set.emplace(u);
                    }
                }
                expansion_set_g.clear();
                std::copy(std::begin(my_set), std::end(my_set), back_inserter(expansion_set_g));
                cout << expansion_set_g.size() << endl;
            }
            // 1st: generate tasks
#pragma omp for
            for (auto i = 0; i < task_hash_table.size(); i++) {
                task_hash_table[i].clear();
            }
#pragma omp for schedule(dynamic, 50) nowait
            for (auto i = 0; i < expansion_set_g.size(); i++) {
                auto a = expansion_set_g[i];
                for (auto b:expansion_pair_lst[a]) {
                    NodePair np(a, b);

                    auto &residual_ref = R[np];
                    double residual_to_push = residual_ref;
                    marker[np] = false;
                    residual_ref -= residual_to_push;
                    P[np] += residual_to_push;

                    for (auto off_a = g.off_out[a]; off_a < g.off_out[a + 1]; off_a++) {
                        auto out_nei_a = g.neighbors_out[off_a];

                        omp_set_lock(&hash_table_lock[out_nei_a]);
                        task_hash_table[out_nei_a].emplace_back(b, residual_to_push);
                        omp_unset_lock(&hash_table_lock[out_nei_a]);
                    }
                }
            }

#pragma omp single
            {
                counter++;
                cout << "gen finished," << counter << endl;
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
                    // push to neighbors (fixed a', varying b' \in N_out(b))
                    auto local_b = task.b_;
                    for (auto off_b = g.off_out[local_b]; off_b < g.off_out[local_b + 1]; off_b++) {
                        auto out_nei_b = g.neighbors_out[off_b];
                        if (out_nei_a != out_nei_b) {
                            NodePair pab(out_nei_a, out_nei_b);

                            // push residual to the pair (a', b')
                            auto &residual_ref = R[pab];
                            residual_ref +=
                                    c * task.residual_ / (g.in_degree(out_nei_a) * g.in_degree(out_nei_b));
                            if (fabs(residual_ref) > r_max) {
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
    }
    cout << "rounds:" << counter << endl;
}

double PFLP::query_P(int a, int b) {
    return P.query(a, b);
}