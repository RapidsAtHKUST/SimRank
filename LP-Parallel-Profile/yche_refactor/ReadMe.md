### Still-Bug

```cpp
void PRLP::local_push(GraphYche &g) {
    vector<std::unordered_map<int, vector<RLPTask>>> thread_local_task_hash_table_lst(num_threads);
    vector<vector<pair<int, vector<RLPTask>>>> thread_local_task_vec_lst(num_threads);
    vector<int> expansion_set_g;
    int counter = 0;
    bool is_go_on;

    cout << "r_max:" << r_max << endl;

#pragma omp parallel
    {
#ifdef HAS_OPENMP
        auto thread_id = omp_get_thread_num();
#else
        auto thread_id = 0;
#endif
        auto &task_hash_table = thread_local_task_hash_table_lst[thread_id];
        auto &task_vec = thread_local_task_vec_lst[thread_id];
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
            if (!is_go_on) { break; }

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
            }
            if (thread_id == 0) {
                cout << "size:" << expansion_set_g.size() << endl;
            }
            // 1st: generate tasks
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
                        task_hash_table[out_nei_a].emplace_back(b, static_cast<float>(residual_to_push),
                                                                a == b);
                    }
                }
            }
            task_vec.clear();
            for (auto &key_val: task_hash_table) { task_vec.emplace_back(std::move(key_val)); }
            task_hash_table.clear();
#pragma omp barrier

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
            for (auto &task_vec_g: thread_local_task_vec_lst) {
#pragma omp for schedule(dynamic, 10)
                for (auto i = 0; i < task_vec_g.size(); i++) {
                    auto a_prime = task_vec_g[i].first;
                    bool is_enqueue = false;

                    for (auto &task :task_vec_g[i].second) {
                        // push to neighbors
                        auto local_b = task.b_;
                        if (task.is_singleton_) {
                            for (auto off_b = g.off_out[local_b]; off_b < g.off_out[local_b + 1]; off_b++) {
                                auto out_nei_b = g.neighbors_out[off_b];
                                if (a_prime < out_nei_b) { // only push to partial pairs for a < local_b
                                    NodePair pab(a_prime, out_nei_b);

                                    // push
                                    auto &res_ref = R[pab];
                                    res_ref += sqrt(2) * c * task.residual_ /
                                               (g.in_degree(a_prime) * g.in_degree(out_nei_b));

                                    if (fabs(res_ref) / sqrt(2) > r_max) { // the criteria for reduced linear system
                                        auto &is_in_q_ref = marker[pab];
                                        if (!is_in_q_ref) {
                                            expansion_pair_lst[a_prime].emplace_back(out_nei_b);
                                            is_enqueue = true;
                                            is_in_q_ref = true;
                                        }
                                    }
                                }
                            }
                        } else {
                            for (auto off_b = g.off_out[local_b]; off_b < g.off_out[local_b + 1]; off_b++) {
                                auto out_nei_a = a_prime;
                                auto out_nei_b = g.neighbors_out[off_b];

                                if (out_nei_a != out_nei_b) {
                                    if (out_nei_a > out_nei_b) {
                                        swap(out_nei_a, out_nei_b);
                                    }
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
                    }
                    if (is_enqueue) { local_expansion_set.emplace_back(a_prime); }
                }
            }
        }
    }

    cout << "rounds:" << counter << endl;
}
```

### Parallel-Naive

```cpp
    vector<pair<int, vector<RLPTask>>> task_vec;
    std::unordered_map<int, vector<RLPTask>> tmp_task_hash_table;
    std::queue<NodePair> Q;
    for (int i = 0; i < n; i++) {
        NodePair np(i, i);
        Q.push(np);
    }
    int counter = 0;
#pragma omp parallel
    {
        vector<NodePair> my_task_vec;

        while (!Q.empty()) {
#pragma omp barrier
            // it remains to run in parallel, currently only sequentially
#pragma omp single
            {
                // 1st: generate tasks
                while (!Q.empty()) {
                    NodePair np = Q.front();
                    Q.pop();
                    marker[np] = false;

                    double residual_to_push = R[np];
                    auto a = np.first;
                    auto b = np.second;
                    if (a == b) { residual_to_push = residual_to_push - r_max / (1 - c); }
                    R[np] -= residual_to_push;
                    P[np] += residual_to_push;

                    // push to neighbors to form tasks
                    for (auto off_a = g.off_out[a]; off_a < g.off_out[a + 1]; off_a++) {
                        auto out_nei_a = g.neighbors_out[off_a];
                        tmp_task_hash_table[out_nei_a].emplace_back(b, static_cast<float>(residual_to_push), a == b);
                    }
                }
                // 2nd: task preparation
                task_vec.clear();
                for (auto &key_val: tmp_task_hash_table) { task_vec.emplace_back(std::move(key_val)); }
                tmp_task_hash_table.clear();

                counter++;
            };

            // 3rd: computation
            cout << task_vec.size() << endl;
#pragma omp for schedule(dynamic, 10)
            for (auto i = 0; i < task_vec.size(); i++) {
                auto a_prime = task_vec[i].first;
                for (auto &task :task_vec[i].second) {
                    auto local_b = task.b_;
                    if (task.is_singleton_) {
                        for (auto off_b = g.off_out[local_b]; off_b < g.off_out[local_b + 1]; off_b++) {
                            auto out_nei_b = g.neighbors_out[off_b];
                            if (a_prime < out_nei_b) { // only push to partial pairs for a < local_b
                                NodePair pab(a_prime, out_nei_b);

                                // push
                                auto &res_ref = R[pab];
                                res_ref +=
                                        sqrt(2) * c * task.residual_ / (g.in_degree(a_prime) * g.in_degree(out_nei_b));

                                if (fabs(res_ref) / sqrt(2) > r_max) { // the criteria for reduced linear system
                                    auto &is_in_q_ref = marker[pab];
                                    if (!is_in_q_ref) {
                                        my_task_vec.emplace_back(pab);
                                        is_in_q_ref = true;
                                    }
                                }
                            }
                        }
                    } else {
                        for (auto off_b = g.off_out[local_b]; off_b < g.off_out[local_b + 1]; off_b++) {
                            auto out_nei_a = a_prime;
                            auto out_nei_b = g.neighbors_out[off_b];

                            if (out_nei_a != out_nei_b) {
                                if (out_nei_a > out_nei_b) {
                                    swap(out_nei_a, out_nei_b);
                                }
                                NodePair pab(out_nei_a, out_nei_b);

                                // push
                                auto &res_ref = R[pab];
                                res_ref += c * task.residual_ / (g.in_degree(out_nei_a) * g.in_degree(out_nei_b));

                                if (fabs(res_ref) / sqrt(2) > r_max) { // the criteria for reduced linear system
                                    auto &is_in_q_ref = marker[pab];
                                    if (!is_in_q_ref) {
                                        my_task_vec.emplace_back(pab);
                                        is_in_q_ref = true;
                                    }
                                }
                            }
                        }
                    }

                }
            }

            // 4th: form the queue and invoke an explicit barrier
#pragma omp critical
            {
                for (auto &task: my_task_vec) {
                    Q.push(std::move(task));
                }
            }
            my_task_vec.clear();
#pragma omp barrier
        }
    }

```
