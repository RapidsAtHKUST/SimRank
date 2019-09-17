#include "reads.h"

#include <cmath>
#include <cstring>

#include <queue>
#include <algorithm>
#include <iostream>
#include <chrono>

#include <boost/format.hpp>

#include "sparsehash/dense_hash_map"

#include "inBuf.h"
#include "outBuf.h"
// #include "meminfo.h"

using google::dense_hash_map;
using namespace std;
using namespace std::chrono;

void reads::loadGraph(char *gName) {
    FILE *fg = fopen(gName, "r");
    if (fg == nullptr) {
        printf("No graph %s\n", gName);
        exit(0);
    }

    Timer tm;
    ef.resize(n);
    eb.resize(n);
    for (int x, y; fscanf(fg, "%d%d", &x, &y) != EOF;) {
        ef[x].push_back(y);
        eb[y].push_back(x);
    }
    rtime = tm.getTime();
    fclose(fg);
}

void reads::constructIndices() {
    vector<pair<int, int>> q;
    vector<int> pos;
    auto cc = int(RAND_MAX * sqrt(c));

    // r samples
    nt.resize(r);
    for (int i = 0; i < r; i++) { nt[i].resize(n); }
    for (int i = 0; i < r; i++) {
        q.resize(0);
        // used for coupling or merging, if pos[p] becomes current level position, a coupling occurs; otherwise not
        // pos[p] current level position of vertex p
        pos.assign(n, -1);

        // 1st: level 0 expansion without stopping
        for (int j = 0, parent; j < n; j++) {
            if (!eb[j].empty()) {
                parent = eb[j][rand() % eb[j].size()];
                // check if vertex parent first occurring for level j+1
                if (pos[parent] < 0) {
                    pos[parent] = q.size();
                    q.emplace_back(parent, -1);
                }
                nt[i][j] = pos[parent];
            } else {
                nt[i][j] = -1;
            }
        }

        // 2nd: further expansion of level [1, t)
        auto q_cur = 0;
        int q_end = q.size();
        for (int j = 0, p, rr; j < t - 1 && q_cur < q_end; j++) {
            for (; q_cur < q_end; q_cur++) {
                auto &in_neighbors = eb[q[q_cur].first];
                if (!in_neighbors.empty() && ((rr = rand()) < cc || j == 0)) {
                    p = in_neighbors[rr % in_neighbors.size()];
                    // check if vertex p first occurring for level j+1
                    if (pos[p] < q_end) {
                        pos[p] = q.size();
                        q.emplace_back(p, -1);
                    }
                    q[q_cur].second = pos[p];
                }
            }
            q_end = q.size();
        }

        // 3rd: link leaf nodes for later query puroose
        auto tmpCnt = 0;
        vector<int> sta, tmp(n);
        for (int j = 0, qid; j < n; j++)
            // check if vertex j has at least one in-neighbor
            if (nt[i][j] > -1) {
                for (sta.resize(1), sta[0] = nt[i][j]; q[*sta.rbegin()].second >= 0;) {
                    sta.push_back(q[*sta.rbegin()].second);
                }
                if ((qid = -q[*sta.rbegin()].second - 2) < 0) {
                    q[*sta.rbegin()].second = -tmpCnt - 2;
                    tmp[tmpCnt] = pos[tmpCnt] = j;
                    tmpCnt++;
                } else {
                    nt[i][tmp[qid]] = j;
                    tmp[qid] = j;
                }
                for (int k = sta.size() - 2; k >= 0; k--) {
                    q[sta[k]].second = q[*sta.rbegin()].second;
                }
            }
        for (int j = 0; j < tmpCnt; j++) {
            nt[i][tmp[j]] = pos[j];
        }
    }

    // randomly shuffle edges
    for (int i = 0; i < n; i++) {
        random_shuffle(ef[i].begin(), ef[i].end());
        random_shuffle(eb[i].begin(), eb[i].end());
    }
}

void reads::serializeForSingleSource(Timer &timer, char *iName) {
    tm.reset();
    outBuf buf(iName);
    // next node of all the leaves
    for (int i = 0; i < r; i++)
        for (int j = 0; j < n; j++)
            buf.insert(nt[i][j]);
    // adjacency list part1: original graph
    for (int i = 0; i < n; i++) {
        buf.insert(ef[i].size());
        for (int j : ef[i])
            buf.insert(j);
    }
    // adjacency list part2: reversed graph
    for (int i = 0; i < n; i++) {
        buf.insert(eb[i].size());
        for (int j : eb[i])
            buf.insert(j);
    }
    rtime += tm.getTime();
}

void reads::postProcessNextForSinglePair() {
    vector<bool> is_visited(n);

    for (auto &next_arr: nt) {
        // bfs to initialize next_arr
        for (auto i = 0; i < n; i++) { is_visited[i] = false; }
        for (auto i = 0; i < n; i++) {
            if (!is_visited[i] && next_arr[i] > -1) {
                // use the first element as the tree id
                auto tree_id = i;
                auto cur_leaf_idx = i;
                do {
                    is_visited[cur_leaf_idx] = true;

                    auto next_leaf_idx = next_arr[cur_leaf_idx];
                    next_arr[cur_leaf_idx] = tree_id;
                    cur_leaf_idx = next_leaf_idx;
                } while (cur_leaf_idx != tree_id);
            }
        }
    }
}

void reads::deserializeForSingleSource(char *iName) {
    inBuf buf(iName);
    ef.resize(n);
    eb.resize(n);
    nt.resize(r);
    for (int i = 0; i < r; i++) {
        nt[i].resize(n);
        for (int j = 0; j < n; j++)
            buf.nextInt(nt[i][j]);
    }
#ifdef SINGLE_PAIR
    postProcessNextForSinglePair();
#endif

    for (int i = 0, s; i < n; i++) {
        buf.nextInt(s);
        ef[i].resize(s);
        for (int j = 0; j < s; j++)
            buf.nextInt(ef[i][j]);
    }
    for (int i = 0, s; i < n; i++) {
        buf.nextInt(s);
        eb[i].resize(s);
        for (int j = 0; j < s; j++)
            buf.nextInt(eb[i][j]);
    }
    rtime = 0;
}

reads::reads(char *gName_, int n_, int r_, double c_, int t_) {
    sprintf(gName, "%s", gName_);
    n = n_, r = r_, c = c_, t = t_;
    t1 = t2 = qCnt = 0;

    char iName[125];
    sprintf(iName, "%s.reads.%d_%d_%lf_%d", gName, n, r, c, t);

#ifdef STORE_INDEX
    if (fopen(iName, "rb") != NULL) {
        // with ready indices: deserialization for index structures
        deserializeForSingleSource(iName);
        return;
    }
#endif

    // 1st: load graph
    loadGraph(gName);

    // 2nd: start constructing indices
    constructIndices();

    // 3rd: serialization for index structures
#ifdef STORE_INDEX
    serializeForSingleSource(tm, iName);
#endif
}


reads::reads(string gName_, int n_, int r_, double c_, int t_) {
    // init parameters
    n = n_, r = r_, c = c_, t = t_;
    t1 = t2 = qCnt = 0;

    // init graph path
    string input_graph_full_path =
            "/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/datasets/edge_list/" + gName_ + ".txt";
    memcpy(gName, input_graph_full_path.c_str(), sizeof(char) * input_graph_full_path.size());
    gName[input_graph_full_path.size()] = '\0';

    // init index path
    string iName = boost::str(boost::format(
            "/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/datasets/reads/%s_%d_%d_%lf_%d.bin") %
                              gName_ % n % r % c % t);

    if (fopen(iName.c_str(), "rb") != nullptr) {
        deserializeForSingleSource(const_cast<char *>(iName.c_str()));
    } else {
        loadGraph(gName);
        auto start = high_resolution_clock::now();
        constructIndices();
        auto end = high_resolution_clock::now();
        cout << "indexing time:" << duration_cast<microseconds>(end - start).count() / pow(10, 6) << " s\n";
        cout << iName << endl;
        serializeForSingleSource(tm, const_cast<char *>(iName.c_str()));
    }
}


reads::~reads() = default;

double reads::queryOne(int x, int y) {
    if (x == y) { return 1; }

    int match_count = 0;
    for (auto i = 0; i < r; i++) {
        // nt[i][x] means no in-neighbor for vertex x
        if (nt[i][x] == nt[i][y] && nt[i][x] != -1 && nt[i][y] != -1) {
            match_count++;
        };
    }
    return static_cast<double>(match_count) * c / r;
}

double reads::queryOne(int x, int y, double *ansVal) {
    if (x == y) { return 1; }
    queryAll(x, ansVal);
    return ansVal[y];
}

void reads::queryAll(int x, double *ansVal) {
    memset(ansVal, 0, sizeof(double) * n);
    if (eb[x].empty()) {
        ansVal[x] = 1;
        return;
    }

#ifdef SHIFT_ARRAY_HASH_TABLE
    if (qCnt >= 20 && t1 < t2 || qCnt < 20 && qCnt % 2 == 0) {
        if (qCnt < 20) tm.reset();
#endif
    dense_hash_map<int, double> sim1;
    sim1.set_empty_key(-1);

    auto *nID = new bool[n];
    memset(nID, 0, sizeof(bool) * n);

    double cc = (1 - c) * r, c1 = r / c;

    int p, q;
    for (auto i = eb[x].begin(); i != eb[x].end(); i++) {
        for (auto j = eb[p = *i].begin(); j != eb[p].end() && j - eb[p].begin() < 10; j++)
            for (auto k = ef[q = *j].begin(); k != ef[q].end() && k - ef[q].begin() < 10; k++)
                if (*k != p)
                    sim1[*k] += cc / eb[p].size() / eb[*k].size();
        sim1[*i] += c1;
    }

    for (int i = 0, cnt, q; i < r; i++) {
        for (int j : eb[x])
            nID[j] = true;

        for (auto j = eb[x].begin(); j != eb[x].end(); j++)
            if (nID[q = *j] && nt[i][q] != q && nt[i][q] != -1) {
                cnt = 0;

                for (int p = nt[i][q]; p != q; p = nt[i][p]) {
                    if (nID[p]) cnt++, nID[p] = false;
                    else sim1[p] += c;
                }

                if (cnt != 0) {
                    cc = c * cnt;
                    for (int p = nt[i][q]; p != q; p = nt[i][p])
                        sim1[p] += cc;
                    sim1[q] += cc;
                }
            }

    }
    cc = c * c / eb[x].size() / r;
    for (auto &it : sim1)
        for (auto jt = ef[p = it.first].begin(); jt != ef[p].end(); jt++)
            ansVal[*jt] += it.second * cc / eb[*jt].size();

    delete[] nID;

#ifdef SHIFT_ARRAY_HASH_TABLE
    if (qCnt++ < 20) t1 += tm.getTime();
} else {

    if (qCnt < 20) tm.reset();

    auto *sim1 = new double[n];
    memset(sim1, 0, sizeof(double) * n);


    auto *nID = new bool[n];
    memset(nID, 0, sizeof(bool) * n);

    double cc = (1 - c) * r, c1 = r / c;

    int p, q;
    for (auto i = eb[x].begin(); i != eb[x].end(); i++) {
        for (auto j = eb[p = *i].begin(); j != eb[p].end() && j - eb[p].begin() < 10; j++)
            for (auto k = ef[q = *j].begin(); k != ef[q].end() && k - ef[q].begin() < 10; k++)
                if (*k != p)
                    sim1[*k] += cc / eb[p].size() / eb[*k].size();
        sim1[*i] += c1;
    }

    for (int i = 0, cnt, q; i < r; i++) {
        for (int j : eb[x])
            nID[j] = true;

        for (auto j = eb[x].begin(); j != eb[x].end(); j++)
            if (nID[q = *j] && nt[i][q] != q && nt[i][q] != -1) {
                cnt = 0;

                for (int p = nt[i][q]; p != q; p = nt[i][p]) {
                    if (nID[p]) cnt++, nID[p] = false;
                    else sim1[p] += c;
                }

                if (cnt != 0) {
                    cc = c * cnt;
                    for (int p = nt[i][q]; p != q; p = nt[i][p])
                        sim1[p] += cc;
                    sim1[q] += cc;
                }
            }

    }

    for (int i = 0; i < n; i++)
        if (sim1[i] > 0)
            for (auto j = ef[i].begin(); j != ef[i].end(); j++)
                ansVal[*j] += sim1[i];

    cc = c * c / r / eb[x].size();
    for (int i = 0; i < n; i++)
        if (!eb[i].empty())
            ansVal[i] = ansVal[i] * cc / eb[i].size();

    delete[] nID;

    if (qCnt++ < 20) t2 += tm.getTime();
}
#endif
}

