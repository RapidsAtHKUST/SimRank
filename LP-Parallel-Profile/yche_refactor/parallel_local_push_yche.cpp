#ifdef HAS_OPENMP

#include <omp.h>

#endif

#include <fstream>

#include <boost/format.hpp>

#include "parallel_local_push_yche.h"

using boost::format;

LP::LP(GraphYche &g, string gName_, double c_, double epsilon_, size_t n_) :
        g_name(gName_), c(c_), epsilon(epsilon_), n(n_) {
    r_max = cal_rmax(c, epsilon);
}

void LP::save() {
    // save data to disk
    // save P
    string p_path = get_file_path_base() + ".P";
    P.save(p_path);

    //save R
    string r_path = get_file_path_base() + ".R";
    R.save(r_path);

    ofstream out;
    // // save meta info
    string meta_path = get_file_path_base() + ".meta";
    out.open(meta_path);
    out << g_name << endl;
    out << n << endl;
    out << epsilon << endl;
    out << c << endl;
    out << -1 << endl;
    out << -1 << endl;
    auto p_size = P.size();
    auto r_size = R.size();
    out << p_size << endl;
    out << r_size << endl;
    out << double(p_size) / (n * n) << endl;
    out << double(r_size) / (n * n) << endl;
    out.close();

    // save exp data
    cout << "save complete" << endl;
}

PFLP::PFLP(GraphYche &g, string name, double c_, double epsilon, size_t n_) : LP(g, name, c_, epsilon, n_) {
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

void PFLP::local_push(GraphYche &g) {
    int counter = 0;
    bool is_go_on;
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
            if (!is_go_on) {
                break;
            }

#pragma omp single
            {
                // aggregation of v_a for expansion, value in thread_local_expansion_set_lst are distinct
                for (auto i = 0; i < thread_local_expansion_set_lst.size(); i++) {
                    prefix_sum[i + 1] = prefix_sum[i] + thread_local_expansion_set_lst[i].size();
                }
                expansion_set_g.resize(static_cast<unsigned long>(prefix_sum.back()));
            }
            std::copy(std::begin(thread_local_expansion_set_lst[thread_id]),
                      std::end(thread_local_expansion_set_lst[thread_id]),
                      std::begin(expansion_set_g) + prefix_sum[thread_id]);

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
#ifdef DEBUG
                cout << "gen finished," << counter << endl;
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

string PFLP::get_file_path_base() {
    return LOCAL_PUSH_DIR + str(format("FLP_%s-%.3f-%.6f") % g_name % c % epsilon);
}
