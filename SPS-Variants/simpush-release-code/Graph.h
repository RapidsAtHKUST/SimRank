#ifndef GRAPH_H
#define GRAPH_H

#include <time.h>
#include <malloc.h>

#include <cstdlib>
#include <cstdio>

#include <algorithm>
#include <iostream>
#include <fstream>

#include <util/util.h>
#include <util/log.h>
#include <util/timer.h>
#include <util/primitives.h>

using namespace std;

#define PAGE_SIZE (4096)
#define IO_REQ_SIZE (PAGE_SIZE * 32)
#define IO_QUEUE_DEPTH (16)

pair<pair<uint32_t, uint32_t> *, size_t> GetEdgeListBin(string &file_path) {
    size_t size = file_size(file_path.c_str());
    size_t num_edges = size / sizeof(uint32_t) / 2;
    vector<pair<uint32_t, uint32_t>> lines;

    using Edge = pair<uint32_t, uint32_t>;
    auto file_fd = open(file_path.c_str(), O_RDONLY | O_DIRECT, S_IRUSR | S_IWUSR);
    Edge *edge_lst = (Edge *) memalign(PAGE_SIZE, size + IO_REQ_SIZE);

    Timer io_timer;
    size_t read_size = 0;
#pragma omp parallel num_threads(IO_QUEUE_DEPTH)
    {
#pragma omp for schedule(dynamic, 1) reduction(+:read_size)
        for (size_t i = 0; i < size; i += IO_REQ_SIZE) {
            auto it_beg = i;
            auto *chars = reinterpret_cast<uint8_t *>(edge_lst);
            auto ret = pread(file_fd, chars + it_beg, IO_REQ_SIZE, it_beg);
            if (ret != IO_REQ_SIZE) {
                log_error("Err, %zu, %zu, %zu, %d", i, it_beg, IO_REQ_SIZE, ret);
            } else {
                read_size += ret;
            }
        }
#pragma omp single
        log_info("%zu, %zu", read_size, size);
    }
    return make_pair(edge_lst, num_edges);
}

class Graph {
public:
    int n;    //number of nodes
    unsigned long long m;    //number of edges
    int **inAdjLists = NULL;
    int **outAdjLists = NULL;
    int *indegree = NULL;
    int *outdegree = NULL;
    double *indegRecip = NULL;

    Graph() {
    }

    ~Graph() {
        if (indegree != NULL) delete[]indegree;
        if (indegRecip != NULL) delete[]indegRecip;
        if (outdegree != NULL) delete[]outdegree;
        if (inAdjLists != NULL) {
            for (int i = 0; i < n; ++i) {
                delete[] inAdjLists[i];
            }
            delete[] inAdjLists;
        }
        if (outAdjLists != NULL) {
            for (int i = 0; i < n; ++i) {
                delete[] outAdjLists[i];
            }
            delete[] outAdjLists;
        }
    }

    void initInDegRecip() {
        indegRecip = new double[n]();
        for (int i = 0; i < n; ++i) {
            if (indegree[i] > 0) indegRecip[i] = 1.0 / (double) indegree[i];
        }
    }

    void mallocAdj(int *&inAdjIdxes, int *&outAdjIdxes) {
        inAdjLists = new int *[n];
        outAdjLists = new int *[n];
        for (int vid = 0; vid < n; ++vid) {
            if (indegree[vid] > 0)
                inAdjLists[vid] = new int[indegree[vid]];
            if (outdegree[vid] > 0)
                outAdjLists[vid] = new int[outdegree[vid]];
        }

        inAdjIdxes = new int[n];
        outAdjIdxes = new int[n];
        for (int i = 0; i < n; i++) {
            inAdjIdxes[i] = 0;
            outAdjIdxes[i] = 0;
        }
    }

    void loadGraph(string filelabel) {
        // 1st: n, m
        cout << "LOADING GRAPH\n";
        string statfilename = "graph/" + filelabel + "_stat.txt";
        cout << statfilename;
        ifstream instatfile(statfilename.c_str());
        if (!instatfile.is_open()) {
            cerr << "file may not exist: " << statfilename << endl;
        }
        instatfile >> n >> m;
        instatfile.close();
        cout << ": n= " << n << " m= " << m << endl;

        // 2nd: cardinality of each neighbor sets
        string degfilename = "graph/" + filelabel + "_deg.txt";
        cout << degfilename;
        ifstream degfile(degfilename.c_str());
        indegree = new int[n];
        outdegree = new int[n];
        int nid = 0;
        while (degfile >> indegree[nid] >> outdegree[nid]) {
            nid++;
        }
        degfile.close();
        cout << ": loaded degree info\n";

        // 3rd: adjacency lists
        int *outAdjIdxes, *inAdjIdxes;
        mallocAdj(inAdjIdxes, outAdjIdxes);
        string datafilename = "graph/" + filelabel + ".txt";
        cout << datafilename;
        ifstream datafile(datafilename.c_str());
        int from;
        int to;
        while (datafile >> from >> to) {
            //update from's outAdjList
            outAdjLists[from][outAdjIdxes[from]] = to;
            outAdjIdxes[from]++;
            //update to's inAdjList
            inAdjLists[to][inAdjIdxes[to]] = from;
            inAdjIdxes[to]++;
        }
        datafile.close();
        delete[]inAdjIdxes;
        delete[]outAdjIdxes;
        cout << ": loaded graph data \n";

        initInDegRecip();
    }

    void loadGraphBinEL(string file_path) {
        // 1st: n, m
        auto edge_lst_size = GetEdgeListBin(file_path);
        auto edge_lst = edge_lst_size.first;
        m = edge_lst_size.second;
        n = 0;
        log_info("Finish Loading");
#pragma omp parallel for reduction(max:n)
        for (size_t i = 0; i < m; ++i) {
            auto edge = edge_lst[i];
            n = max<int>(n, edge.first);
            n = max<int>(n, edge.second);
        }
        n += 1;
        log_info("total vertex#: %d", n);
        log_info("total edge#: %zu", m);

        // 2nd: cardinality of each neighbor sets
        indegree = new int[n];
        outdegree = new int[n];
#pragma omp parallel
        {
            Histogram(m, indegree, n, [edge_lst](size_t i) {
                return edge_lst[i].second;
            });
            Histogram(m, outdegree, n, [edge_lst](size_t i) {
                return edge_lst[i].first;
            });
        };

        // 3rd: adj
        int *outAdjIdxes, *inAdjIdxes;
        mallocAdj(inAdjIdxes, outAdjIdxes);

        auto *locks = (omp_lock_t *) malloc(sizeof(omp_lock_t) * n);
        for (auto i = 0; i < n; i++) {
            omp_init_lock(&locks[i]);
        }
#pragma omp parallel for
        for (size_t i = 0; i < m; ++i) {
            int src, dst;
            std::tie(src, dst) = edge_lst[i];
            omp_set_lock(&locks[src]);
            outAdjLists[src][outAdjIdxes[src]] = dst;
            outAdjIdxes[src]++;
            omp_unset_lock(&locks[src]);

            omp_set_lock(&locks[dst]);
            inAdjLists[dst][inAdjIdxes[dst]] = src;
            inAdjIdxes[dst]++;
            omp_unset_lock(&locks[dst]);
        }
        for (auto i = 0; i < n; i++) {
            omp_destroy_lock(&locks[i]);
        }
        delete[]inAdjIdxes;
        delete[]outAdjIdxes;
        cout << ": loaded graph data \n";
        free(locks);
        log_info("Finish CSR");

#pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            sort(outAdjLists[i], outAdjLists[i] + outdegree[i]);
            sort(inAdjLists[i], inAdjLists[i] + indegree[i]);
        }
        log_info("Finish Sort");
        free(edge_lst);

        initInDegRecip();
    }
};


#endif
