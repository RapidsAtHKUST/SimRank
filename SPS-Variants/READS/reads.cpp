#include "reads.h"

#include <cmath>
#include <cstring>

#include <queue>
#include <algorithm>
#include <iostream>

#include "sparsehash/dense_hash_map"

#include "inBuf.h"
#include "outBuf.h"
// #include "meminfo.h"

using google::dense_hash_map;
using namespace std;

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
    int q0, q1;
    vector<pair<int, int>> q;
    vector<int> pos;
    auto cc = int(RAND_MAX * sqrt(c));
    vector<int> sta, tmp(n);
    int tmpCnt;

    // r samples
    nt.resize(r);
    for (int i = 0; i < r; i++) { nt[i].resize(n); }
    for (int i = 0; i < r; i++) {
        q.resize(0);
        pos.assign(n, -1);

        // 1st
        for (int j = 0, p; j < n; j++) {
            if (!eb[j].empty()) {
                if (pos[p = eb[j][rand() % eb[j].size()]] < 0) {
                    pos[p] = q.size();
                    q.emplace_back(p, -1);
                }
                nt[i][j] = pos[p];
            } else {
                nt[i][j] = -1;
            }
        }

        // 2nd
        q0 = 0;
        q1 = q.size();
        for (int j = 0, p, rr; j < t - 1 && q0 < q1; j++) {
            for (; q0 < q1; q0++) {
                if (!eb[q[q0].first].empty() && ((rr = rand()) < cc || j == 0)) {
                    p = eb[q[q0].first][rr % eb[q[q0].first].size()];
                    if (pos[p] < q1) {
                        pos[p] = q.size();
                        q.emplace_back(p, -1);
                    }
                    q[q0].second = pos[p];
                }
            }
            q1 = q.size();
        }

        // 3rd
        tmpCnt = 0;
        for (int j = 0, qid; j < n; j++)
            if (nt[i][j] > -1) {
                for (sta.resize(1), sta[0] = nt[i][j]; q[*sta.rbegin()].second >= 0;)
                    sta.push_back(q[*sta.rbegin()].second);

                if ((qid = -q[*sta.rbegin()].second - 2) < 0) {
                    q[*sta.rbegin()].second = -tmpCnt - 2;
                    tmp[tmpCnt] = pos[tmpCnt] = j;
                    tmpCnt++;
                } else {
                    nt[i][tmp[qid]] = j;
                    tmp[qid] = j;
                }
                for (int k = sta.size() - 2; k >= 0; k--)
                    q[sta[k]].second = q[*sta.rbegin()].second;
            }
        for (int j = 0; j < tmpCnt; j++)
            nt[i][tmp[j]] = pos[j];
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
    vector<bool> is_visited(n, false);
    for (auto &next_arr: nt) {
        // bfs to initialize next_arr
        for (auto i = 0; i < n; i++) {
            if (!is_visited[i] && next_arr[i] >= 0) {
                // use the first element as the tree id
                auto tree_id = next_arr[i];
                is_visited[tree_id] = true;

                auto cur_leaf_idx = tree_id;
                int next_leaf_idx;
                while (next_arr[cur_leaf_idx] != tree_id) {
                    is_visited[cur_leaf_idx] = true;

                    next_leaf_idx = next_arr[cur_leaf_idx];
                    next_arr[cur_leaf_idx] = tree_id;
                    cur_leaf_idx = next_leaf_idx;
                }
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

reads::~reads() = default;

double reads::queryOne(int x, int y) {
    if (x == y) { return 1; }

    int match_count = 0;
    for (auto i = 0; i < r; i++) {
        if (nt[i][x] == nt[i][y]) {
            cout << nt[i][x] << ", " << nt[i][y] << endl;
            match_count++;
        };
    }
    return static_cast<double>(match_count) * c / r;
}

void reads::queryAll(int x, double *ansVal) {
    memset(ansVal, 0, sizeof(double) * n);
    if (eb[x].empty()) {
        ansVal[x] = 1;
        return;
    }

    if (qCnt >= 20 && t1 < t2 || qCnt < 20 && qCnt % 2 == 0) {
        if (qCnt < 20) tm.reset();

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
}