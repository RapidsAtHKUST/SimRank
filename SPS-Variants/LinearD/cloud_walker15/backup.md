header

```cpp
    void mcap(); // Monte Carlo All-pair SimRank computation

    void save();
    
    void load();

```

impl cpp

```cpp
void CloudWalker::mcap() {
    auto start = std::chrono::high_resolution_clock::now();
    sim.resize(n, n);
    sim.setZero();
    VectorXd tmp(n);
    for (size_t i = 0; i < n; i++) {
        tmp.setZero();
        if (i % 100 == 0) {
            cout << str(format("computing single source for %s/%s") % i % n) << endl;
        }
        mcss(i, tmp);
        sim.row(i) = tmp;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    cpu_time = elapsed.count();
    mem_size = getValue();
}

void CloudWalker::save() {
    cout << "saving to disk..." << endl;
    write_binary((get_file_path_base() + ".bin").c_str(), sim);
    ofstream out(get_file_path_base() + ".meta");
    out << cpu_time << endl;
    out << mem_size << endl;
    out << n << endl;
    out.close();
}

void CloudWalker::load() {
    read_binary((get_file_path_base() + ".bin").c_str(), sim);
}

```

main

```cpp

void test_cloud_walker(string data_name, double c = 0.6, int T = 10, int L = 3, int R = 100, int R_prime = 10000) {
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    srand(static_cast<unsigned int>(time(nullptr))); // random number generator
    std::chrono::duration<double> elapsed{};
    int sample_size = 1000;

    // 1st: indexing
    auto start = std::chrono::high_resolution_clock::now();
    CloudWalker cw(&g, data_name, c, T, L, R, R_prime);
    auto pre_time = std::chrono::high_resolution_clock::now();

    // output to log file
    size_t n = num_vertices(g);
    ofstream out(CLOUD_WALKER_DIR + cw.g_name + string(".meta")); // the file to store the execute info
    elapsed = pre_time - start;
    out << elapsed.count() << endl; // record the pre-processing time
    out << n << endl; // number of vertices

    // 2nd: query
    for (int i = 0; i < sample_size; i++) {
        auto source_node = static_cast<int>(rand() % n);
        VectorXd tmp(n);
        auto start_ss = std::chrono::high_resolution_clock::now();
        cw.mcss(source_node, tmp);
        auto end_ss = std::chrono::high_resolution_clock::now();
        elapsed = end_ss - start_ss;
        auto mem_size = cw.mem_size; // record the memory size
        auto cpu_time = elapsed.count();
        out << cpu_time << " " << mem_size << endl;
    }
    out.close();
}
```