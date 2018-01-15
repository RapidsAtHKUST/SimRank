//
// Created by yche on 12/17/17.
//
#include "sling.h"

#include <boost/format.hpp>

#include "ground_truth/stat.h"
#include "ground_truth/graph_yche.h"
#include "ground_truth/yche_serialization.h"

const double Sling::BACKEPS = 7.28e-4; //EPS / 23.;
const double Sling::K = 10.;

int Sling::NUMTHREAD = std::thread::hardware_concurrency();

using namespace std::chrono;

void Sling::init() {
    first = new bool[g->n];
    second = new bool[g->n];
    memset(first, 0, sizeof(bool) * g->n);
    memset(second, 0, sizeof(bool) * g->n);
    for (int u = 0; u < g->n; ++u) {
        if (!(first[u] = (g->inedge[u].size() <= K / BACKEPS))) {
            second[u] = false;
            continue;
        }
        int cnt = 0;
        for (auto itr = g->inedge[u].begin(); itr != g->inedge[u].end(); ++itr) {
            cnt += g->inedge[*itr].size();
        }
        second[u] = (cnt <= K / BACKEPS);
    }
    sqrtct[0] = 1.;
    for (int i = 1; i < 20; ++i)
        sqrtct[i] = sqrtc * sqrtct[i - 1];
}

//########################### indexing 1st: Diagonal Correction Matrix ####################################
double Sling::calcDi(int i, double eps, bool &early, int &R, int tid) {
    R = 0;
    early = true;
    if (g->inedge[i].size() == 0) return 1.;
    if (g->inedge[i].size() == 1) {
        if (g->inedge[i][0] == i) return 1.;
        else return 1. - c;
    }
    return calcDi_1(i, eps, early, R, tid);
}

double Sling::calcDi_1(int i, double eps, bool &early, int &R, int tid) {
    const auto &in_i = g->inedge[i];
    const vector<int> *inedge = g->inedge;
    const int isize = in_i.size();
    eps = eps * (double) isize / (double) (isize - 1) / c;
//    int Rs = 14. / 3. / eps * 2. * log(2 * g->n) / log(2.71828);
    constexpr double failure_probability = 0.01;

    int Rs = 14. / 3. / eps * log(4.0 * g->n / failure_probability) / log(2.71828);
    int X = 0;
    double cc = c * (1l << 32);
    for (; R < Rs; ++R) {
        int rx = gen.rand(tid) % isize;
        int ry = gen.rand(tid) % (isize - 1);
        if (ry >= rx) ++ry;
        int x = in_i[rx];
        int y = in_i[ry];
        do {
            if (inedge[x].empty() || inedge[y].empty())
                break;
            x = inedge[x][gen.rand(tid) % inedge[x].size()];
            y = inedge[y][gen.rand(tid) % inedge[y].size()];
            if (x == y) {
                ++X;
                break;
            }
        } while (gen.rand(tid) < cc);
    }
    if (X / (double) R <= eps) {
        return 1. - c * c * (X / (double) R) * (double) (isize - 1) / (double) isize - c / (double) isize;
    }
    double upp = X / (double) R + sqrt(eps * X / (double) R);
//    int Rl = (2 * upp + 2. / 3. * eps) / (eps * eps) * log(2 * g->n) / log(2.71828);
    int Rl = (2 * upp + 2. / 3. * eps) / (eps * eps) * log(4.0 * g->n / failure_probability) / log(2.71828);
    for (; R < Rl; ++R) {
        int rx = gen.rand(tid) % isize;
        int ry = gen.rand(tid) % (isize - 1);
        if (ry >= rx) ++ry;
        int x = in_i[rx];
        int y = in_i[ry];
        do {
            if (inedge[x].empty() || inedge[y].empty())
                break;
            x = inedge[x][gen.rand(tid) % inedge[x].size()];
            y = inedge[y][gen.rand(tid) % inedge[y].size()];
            if (x == y) {
                ++X;
                break;
            }
        } while (gen.rand(tid) < cc);
    }
    early = false;
    return 1. - c * c * (X / (double) R) * (double) (isize - 1) / (double) isize - c / (double) isize;
}

void Sling::t_calcD(double eps, mutex *lock, int *cursor, int tid) {
    int i, u;
    while (true) {
        lock->lock();
        i = *cursor;
        u = ((*cursor) += BLOCKSIZE);
        lock->unlock();
        if (i >= g->n)
            return;
        if (u > g->n) u = g->n;
        for (; i < u; ++i) {
            int RWCNT;
            bool early;
            double di = calcDi(i, eps / c, early, RWCNT, tid);
            d[i] = di;
        }
    }
}

void __Sling_t_calcD(Sling *sim, double eps, mutex *lock, int *cursor, int tid) {
    sim->t_calcD(eps, lock, cursor, tid);
}

void Sling::calcD(double eps) {
    if (d != NULL) delete[] d;
    d = new double[g->n];
    int cursor = 0;
    mutex lock;
    vector<thread> threads;
    for (int i = 0; i < NUMTHREAD - 1; ++i)
        threads.emplace_back(__Sling_t_calcD, this, eps, &lock, &cursor, i);
    t_calcD(eps, &lock, &cursor, NUMTHREAD - 1);
    for (int t = 0; t < NUMTHREAD - 1; ++t)
        threads[t].join();
    d_bar = 0.;
    for (int i = 0; i < g->n; ++i)
        d_bar += d[i];
    d_bar /= g->n;
}

//########################### indexing 2nd: Backward Propagation ####################################
map<pair<int, int>, double, PairCmp> Sling::pushback(int u, double eps, int tid) {
    map<pair<int, int>, double, PairCmp> p;
    gmap<int, double> pt;
    gmap<int, double> ptt;
    gset<int> s;
    deque<int> q;
    deque<int> qq;

    int t = 0;
    pt[u] = 1.;
    q.push_back(u);

    while (!q.empty()) {
        int v = q.front();
        double puvt = pt[v];;
        p[make_pair(v, t)] = puvt;
        for (unsigned i = 0; i < g->edge[v].size(); ++i) {
            int vv = g->edge[v][i];
            double x = ptt[vv] += puvt * sqrtc / (double) g->inedge[vv].size();
            if (x > eps)// || (t < 2 && x > eps / K))
            {
                if (s.find(vv) == s.end()) {
                    qq.push_back(vv);
                    s.insert(vv);
                }
            }
        }
        q.pop_front();
        if (q.empty()) {
            s.clear();
            pt.swap(ptt);
            ptt.clear();
            q.swap(qq);
            qq.clear();
            ++t;
        }
    }
    p.erase(make_pair(u, 0));
    return p;
}

void Sling::t_backward(double eps, mutex *tasklock, int *cursor, int tid, mutex *plock) {
    int s, t;
    vector<tuple<int, int, int, double>> vec;
    while (true) {
        vec.clear();
        tasklock->lock();
        s = *cursor;
        t = ((*cursor) += BLOCKSIZE);
        tasklock->unlock();

        if (s >= g->n) return;
        if (t > g->n) t = g->n;

        for (int v = s; v < t; ++v) {
            auto pv = pushback(v, eps, tid);
            for (auto itr = pv.begin(); itr != pv.end(); ++itr) {
                if (itr->second >= eps || (itr->first.second < 2 && itr->second >= eps / K)) {
                    int u = itr->first.first;
                    int t = itr->first.second;
                    double value = itr->second;
                    if (t == 1 && first[u]) continue;
                    if (t == 2 && second[u]) continue;

                    p.push_back(make_tuple(u, t, v, value));
                }
            }
        }
    }
}

void __Sling_t_backward(Sling *sim, double eps, mutex *tasklock, int *cursor, int tid, mutex *plock) {
    sim->t_backward(eps, tasklock, cursor, tid, plock);
}

void Sling::backward(double eps) {
    Sling::NUMTHREAD = 1;
    int plockNum = (g->n - 1) / BLOCKSIZE + 1;
    auto *plock = new mutex[plockNum];
    mutex tasklock;
    int cursor = 0;
    if (!p.empty()) { p.clear(); }
    p.reserve(2339768660l);

    vector<thread> threads;
    for (int i = 0; i < NUMTHREAD - 1; ++i)
        threads.emplace_back(__Sling_t_backward, this, eps, &tasklock, &cursor, i, plock);
    t_backward(eps, &tasklock, &cursor, NUMTHREAD - 1, plock);
    for (int t = 0; t < NUMTHREAD - 1; ++t)
        threads[t].join();
    delete[] plock;
    cerr << "sort" << endl;
    sort(p.begin(), p.end(), cmpTuple);
    cerr << "sort finished" << endl;
    pstart.resize(g->n + 1);
    pstart[0] = 0;
    int x = 1;
    for (long long i = 0; i < p.size(); ++i) {
        if (std::get<0>(p[i]) >= x) {
            for (; x <= std::get<0>(p[i]); ++x)
                pstart[x] = i;
        }
    }
    for (; x <= g->n; ++x)
        pstart[x] = p.size();
}

///----------------------------- 1st: single pair
double Sling::simrank(int u, int v) {
    assert(!p.empty() && d != nullptr);
    if (u == v) return 1.;
    google::sparse_hash_map<pair<int, int>, double, PairHash> pu;
    gmap<pair<int, int>, double, PairHash> pv;
    gmap<pair<int, int>, double, PairHash> ppu;
    gmap<pair<int, int>, double, PairHash> ppv;
    for (long long i = pstart[u]; i < pstart[u + 1]; ++i) {
        pu[make_pair(std::get<1>(p[i]), std::get<2>(p[i]))] = std::get<3>(p[i]);
    }
    for (long long i = pstart[v]; i < pstart[v + 1]; ++i) {
        pv[make_pair(std::get<1>(p[i]), std::get<2>(p[i]))] = std::get<3>(p[i]);
    }

    int cnt = 0;
    int thr = 1. / sqrt(BACKEPS);
    for (long long i = pstart[u]; i < pstart[u + 1] && cnt < thr; ++i) {
        int t = std::get<1>(p[i]);
        int x = std::get<2>(p[i]);
        if (g->inedge[x].size() > thr || g->inedge[x].empty()) continue;
        ++cnt;
        for (auto xitr = g->inedge[x].begin(); xitr != g->inedge[x].end(); ++xitr) {
            if (pu.find(make_pair(t + 1, *xitr)) == pu.end())
                ppu[make_pair(t + 1, *xitr)] += std::get<3>(p[i]) * c / g->inedge[x].size();
        }
    }
    cnt = 0;
    for (long long i = pstart[v]; i < pstart[v + 1] && cnt < thr; ++i) {
        int t = std::get<1>(p[i]);
        int x = std::get<2>(p[i]);
        if (g->inedge[x].size() > thr || g->inedge[x].empty()) continue;
        ++cnt;
        for (auto xitr = g->inedge[x].begin(); xitr != g->inedge[x].end(); ++xitr) {
            if (pv.find(make_pair(t + 1, *xitr)) == pv.end())
                ppv[make_pair(t + 1, *xitr)] += std::get<3>(p[i]) * c / g->inedge[x].size();
        }
    }
    for (auto itr = ppu.begin(); itr != ppu.end(); ++itr) {
        pu[itr->first] = itr->second;
    }
    for (auto itr = ppv.begin(); itr != ppv.end(); ++itr) {
        pv[itr->first] = itr->second;
    }
    if (first[u]) {
        for (auto itr = g->inedge[u].begin(); itr != g->inedge[u].end(); ++itr) {
            pu[make_pair(1, *itr)] = sqrtc / g->inedge[u].size();
        }
    }
    if (second[u]) {
        for (auto uitr = g->inedge[u].begin(); uitr != g->inedge[u].end(); ++uitr) {
            int v = *uitr;
            for (auto vitr = g->inedge[v].begin(); vitr != g->inedge[v].end(); ++vitr) {
                pu[make_pair(2, *vitr)] += c / g->inedge[u].size() / g->inedge[v].size();
            }
        }
    }
    if (first[v]) {
        for (auto itr = g->inedge[v].begin(); itr != g->inedge[v].end(); ++itr) {
            pv[make_pair(1, *itr)] = sqrtc / g->inedge[v].size();
        }
    }
    if (second[v]) {
        for (auto vitr = g->inedge[v].begin(); vitr != g->inedge[v].end(); ++vitr) {
            int vv = *vitr;
            for (auto vitr = g->inedge[vv].begin(); vitr != g->inedge[vv].end(); ++vitr) {
                pv[make_pair(2, *vitr)] += c / g->inedge[v].size() / g->inedge[vv].size();
            }
        }
    }
    double sim = 0.;
    for (auto uitr = pu.begin(); uitr != pu.end(); ++uitr) {
        auto vitr = pv.find(uitr->first);
        if (vitr == pv.end())
            continue;
        sim += uitr->second * d[uitr->first.second] * vitr->second;
    }
    return sim;
}

// TODO
///----------------------------- 2nd: single source
vector<double> Sling::simrank(int u) {
    assert(!p.empty() && d != NULL);
    map<pair<int, int>, double, PairCmp> pu;
    for (long long i = pstart[u]; i < pstart[u + 1]; ++i)
        pu[make_pair(std::get<1>(p[i]), std::get<2>(p[i]))] = std::get<3>(p[i]);
    gmap<pair<int, int>, double, PairHash> ppu;
    int cnt = 0;
    int thr = 1. / sqrt(BACKEPS);
    for (long long i = pstart[u]; i < pstart[u + 1] && cnt < thr; ++i) {
        int t = std::get<1>(p[i]);
        int x = std::get<2>(p[i]);
        if (g->inedge[x].size() > thr || g->inedge[x].empty()) continue;
        ++cnt;
        for (auto xitr = g->inedge[x].begin(); xitr != g->inedge[x].end(); ++xitr) {
            if (pu.find(make_pair(t + 1, *xitr)) == pu.end())
                ppu[make_pair(t + 1, *xitr)] += std::get<3>(p[i]) * c / g->inedge[x].size();
        }
    }
    for (auto itr = ppu.begin(); itr != ppu.end(); ++itr) {
        pu[itr->first] = itr->second;
    }
    if (second[u]) {
        for (auto uitr = g->inedge[u].begin(); uitr != g->inedge[u].end(); ++uitr) {
            int v = *uitr;
            for (auto vitr = g->inedge[v].begin(); vitr != g->inedge[v].end(); ++vitr) {
                pu[make_pair(2, *vitr)] += c / g->inedge[u].size() / g->inedge[v].size();
            }
        }
    }
    if (first[u]) {
        for (auto itr = g->inedge[u].begin(); itr != g->inedge[u].end(); ++itr) {
            pu[make_pair(1, *itr)] = sqrtc / g->inedge[u].size();
        }
    }
    vector<double> sim(g->n, 0.);
    vector<double> q;
    vector<double> qq;
    deque<int> s;
    deque<int> ss;
    for (int t = 1;; ++t) {
        auto start = pu.lower_bound(make_pair(t, 0));
        if (start == pu.end())
            break;
        auto end = pu.lower_bound(make_pair(t + 1, 0));
        if (start == end) continue;
        q.clear();
        q.resize(g->n, 0.);
        qq.clear();
        qq.resize(g->n, 0.);
        s.clear();
        ss.clear();
        int TTT = g->n / 10;
        for (; start != end; ++start) {
            q[start->first.second] = start->second * d[start->first.second];
            s.push_back(start->first.second);
        }
        double eps = sqrtct[t] * BACKEPS;
        for (int l = 1; l < t; ++l) {
            if (s.size() > TTT) {
                for (int u = 0; u < g->n; ++u) {
                    if (q[u] <= eps) continue;
                    for (auto vitr = g->edge[u].begin(); vitr != g->edge[u].end(); ++vitr) {
                        int v = *vitr;
                        qq[v] += sqrtc * q[u] / (double) g->inedge[v].size();
                    }
                }
                q.swap(qq);
                qq.clear();
                qq.resize(g->n, 0.);
            } else {
                for (auto uitr = s.begin(); uitr != s.end(); ++uitr) {
                    int u = *uitr;
                    for (auto vitr = g->edge[u].begin(); vitr != g->edge[u].end(); ++vitr) {
                        int v = *vitr;
                        double inc = sqrtc * q[u] / (double) g->inedge[v].size();
                        double x = qq[v] += inc;
                        if (x > eps && x <= eps + inc) {
                            ss.push_back(v);
                        }
                    }
                }
                q.swap(qq);
                qq.clear();
                qq.resize(g->n, 0.);
                s.swap(ss);
                ss.clear();
            }
        }
        if (s.size() > TTT) {
            for (int u = 0; u < g->n; ++u) {
                if (q[u] <= eps) continue;
                for (auto vitr = g->edge[u].begin(); vitr != g->edge[u].end(); ++vitr) {
                    int v = *vitr;
                    sim[v] += sqrtc * q[u] / (double) g->inedge[v].size();
                }
            }
        } else {
            for (auto uitr = s.begin(); uitr != s.end(); ++uitr) {
                int u = *uitr;
                for (auto vitr = g->edge[u].begin(); vitr != g->edge[u].end(); ++vitr) {
                    int v = *vitr;
                    sim[v] += sqrtc * q[u] / (double) g->inedge[v].size();
                }
            }
        }
    }

    sim[u] = 1.;
    return sim;
}

string Sling::get_file_path_base() {
    return SLING_INDEX_DIR + "/" + boost::str(boost::format("RLP_%s-%.3f-%.6f-%.6f") % g_name % c % eps_d % theta);
}

void Sling::build_or_load_index() {
    string d_file_path = get_file_path_base() + ".d";
    string p_file_path = get_file_path_base() + ".p";
    string pstart_file_path = get_file_path_base() + ".pstart";
    if (file_exists(d_file_path) && file_exists(p_file_path) && file_exists(pstart_file_path)) {
        cout << "indexing exists......." << endl;
        YcheSerializer serializer;
        cout << d_file_path << endl;
        cout << p_file_path << endl;
        FILE *pFile = fopen(d_file_path.c_str(), "r");
        size_t tmp;
        serializer.read_array_into_ref(pFile, d, tmp);
        cout << boost::format("d size: %s") % tmp << endl;
        fclose(pFile);

        FILE *pFile2 = fopen(p_file_path.c_str(), "r");
        serializer.read_tuple_vec(pFile2, p);
        fclose(pFile2);
        cout << boost::format("p size: %s") % p.size() << endl;

        FILE *pFile3 = fopen(pstart_file_path.c_str(), "r");
        serializer.read_vec(pFile3, pstart);
        fclose(pFile3);
    } else {
        //  build the index
        cout << "indexing..." << endl;

        auto tmp_start = std::chrono::high_resolution_clock::now();
        calcD(eps_d);
        auto tmp_end = std::chrono::high_resolution_clock::now();

        cout << "finish calcD " << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6))
             << " s\n";
        tmp_start = std::chrono::high_resolution_clock::now();
        backward(theta);
        tmp_end = std::chrono::high_resolution_clock::now();

        cout << "finish backward " << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6))
             << " s\n";
        cout << "mem size:" << getValue() << endl;

        cout << "store index......." << endl;
        // store indexing d
        YcheSerializer serializer;
        FILE *pFile = fopen(d_file_path.c_str(), "wb");
        serializer.write_array(pFile, d, static_cast<size_t>(g->n));
        fclose(pFile);
        // store indexing p
        FILE *pFile2 = fopen(p_file_path.c_str(), "wb");
        serializer.write_tuple_vec(pFile2, p);
        fclose(pFile2);
        // store indexing pstart

        FILE *pFile3 = fopen(pstart_file_path.c_str(), "wb");
        serializer.write_vec(pFile3, pstart);
        fclose(pFile2);
    }
}
