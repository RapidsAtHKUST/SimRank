//
// Created by yche on 12/17/17.
//
#include "sling.h"

const double Sling::EPS = .025;
const double Sling::BACKEPS = 7.28e-4; //EPS / 23.;
const double Sling::DEPS = 5e-3; //EPS * 3. * 3. / 23.;
const double Sling::K = 10.;
int Sling::NUMTHREAD = std::thread::hardware_concurrency();
//int Sling::NUMTHREAD = 1;

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

//###########################################################################################
bool Sling::trial(int u, int v, int tid) {
    if (u == v) return true;
    do {
        if (g->inedge[u].empty() || g->inedge[v].empty())
            return false;
        u = g->inedge[u][gen.rand(tid) % g->inedge[u].size()];
        v = g->inedge[v][gen.rand(tid) % g->inedge[v].size()];
        if (u == v) return true;
    } while (gen.rand(tid) < c * (1l << 32));
    return false;
}

//########################### Diagonal Correction Matrix ####################################
double Sling::calcDi(int i, double eps, bool &early, int &R, int tid) {
    R = 0;
    early = true;
    if (g->inedge[i].size() == 0) return 1.;
    if (g->inedge[i].size() == 1) {
        if (g->inedge[i][0] == i) return 1.;
        else return 1. - c;
    }
//    return calcDi_0(i, eps, early, R, tid);
    return calcDi_1(i, eps, early, R, tid);
//    if (second[i])
//        return calcDi_2(i, eps, early, R, tid);
//    else
//        return calcDi_1(i, eps, early, R, tid);
}

double Sling::calcDi_0(int i, double eps, bool &early, int &R, int tid) {
    const auto &in_i = g->inedge[i];
    const vector<int> *inedge = g->inedge;
    const int isize = in_i.size();
    eps = eps * (double) isize / (double) (isize - 1) / c;
    int Rs = (2. + eps) / eps / eps * 2. * log(sqrt(2) * g->n) / log(2.71828);
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
    return 1. - c * c * (X / (double) R) * (double) (isize - 1) / (double) isize - c / (double) isize;
}

double Sling::calcDi_1(int i, double eps, bool &early, int &R, int tid) {
    const auto &in_i = g->inedge[i];
    const vector<int> *inedge = g->inedge;
    const int isize = in_i.size();
    eps = eps * (double) isize / (double) (isize - 1) / c;
//    int Rs = 14. / 3. / eps * 2. * log(2 * g->n) / log(2.71828);
    constexpr double failure_probability = 0.01;
//    constexpr double failure_probability = 0.0001;
//    constexpr double failure_probability = 0.01;

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

double Sling::calcDi_2(int i, double eps, bool &early, int &R, int tid) {
    auto &in_i = g->inedge[i];
    double p = 0.;
    gmap<int, double> pp;
    for (auto uitr = in_i.begin(); uitr != in_i.end(); ++uitr) {
        int u = *uitr;
        for (auto vitr = g->inedge[u].begin(); vitr != g->inedge[u].end(); ++vitr) {
            int v = *vitr;
            double q = 1. / (in_i.size() * g->inedge[u].size());
            p += 2. * q * pp[v];
            pp[v] += q;
        }
    }
    eps = eps * (double) in_i.size() / (double) (in_i.size() - 1) / c / c;
    int Rs = 14. / 3. / eps * 2. * log(2 * g->n) / log(2.71828);
    int X = 0;
    for (; R < Rs; ++R) {
        int rx = gen.rand(tid) % in_i.size();
        int ry = gen.rand(tid) % (in_i.size() - 1);
        if (ry >= rx) ++ry;
        int x = in_i[rx];
        int y = in_i[ry];
        if (g->inedge[x].empty() || g->inedge[y].empty())
            continue;
        x = g->inedge[x][gen.rand(tid) % g->inedge[x].size()];
        y = g->inedge[y][gen.rand(tid) % g->inedge[y].size()];
        if (x == y)
            continue;
        do {
            if (g->inedge[x].empty() || g->inedge[y].empty())
                break;
            x = g->inedge[x][gen.rand(tid) % g->inedge[x].size()];
            y = g->inedge[y][gen.rand(tid) % g->inedge[y].size()];
            if (x == y) {
                ++X;
                break;
            }
        } while (gen.rand(tid) < c * (1l << 32));
    }
    if (X / (double) R <= eps) {
        return 1. - c * c * c * (X / (double) R) * (double) (in_i.size() - 1) / (double) in_i.size() - c * c * p -
               c / (double) in_i.size();
    }
    double upp = X / (double) R + sqrt(eps * X / (double) R);
    int Rl = (2 * upp + eps) / (eps * eps) * 2. * log(2 * g->n) / log(2.71828);
    for (; R < Rl; ++R) {
        int rx = gen.rand(tid) % in_i.size();
        int ry = gen.rand(tid) % (in_i.size() - 1);
        if (ry >= rx) ++ry;
        int x = in_i[rx];
        int y = in_i[ry];
        if (g->inedge[x].empty() || g->inedge[y].empty())
            continue;
        x = g->inedge[x][gen.rand(tid) % g->inedge[x].size()];
        y = g->inedge[y][gen.rand(tid) % g->inedge[y].size()];
        if (x == y)
            continue;
        do {
            if (g->inedge[x].empty() || g->inedge[y].empty())
                break;
            x = g->inedge[x][gen.rand(tid) % g->inedge[x].size()];
            y = g->inedge[y][gen.rand(tid) % g->inedge[y].size()];
            if (x == y) {
                ++X;
                break;
            }
        } while (gen.rand(tid) < c * (1l << 32));
    }
    early = false;
    return 1. - c * c * c * (X / (double) R) * (double) (in_i.size() - 1) / (double) in_i.size() - c * c * p -
           c / (double) in_i.size();
}

void Sling::t_calcD(double eps, mutex *lock, int *cursor, int tid) {
    int i, u;
    while (true) {
        lock->lock();
        i = *cursor;
        u = ((*cursor) += BLOCKSIZE);
//        cerr << "Thread " << tid << " gets tasks " << i << " to " << u - 1 << endl;
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

//########################### Backward Propagation ####################################
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
//        cerr << "Thread " << tid << " gets tasks " << s << " to " << t - 1 << endl;
        tasklock->unlock();
        if (s >= g->n) return;
        if (t > g->n) t = g->n;
//        map<int, map<pair<int, int>, double, Sling::PairCmp> >* uvtp = new map<int, map<pair<int, int>, double, Sling::PairCmp> >();
        for (int v = s; v < t; ++v) {
            auto pv = pushback(v, eps, tid);
            for (auto itr = pv.begin(); itr != pv.end(); ++itr) {
                if (itr->second >= eps || (itr->first.second < 2 && itr->second >= eps / K)) {
                    int u = itr->first.first;
                    int t = itr->first.second;
                    double value = itr->second;
                    if (t == 1 && first[u]) continue;
                    if (t == 2 && second[u]) continue;
//                    vec.push_back(make_tuple(u, t, v, value));
                    p.push_back(make_tuple(u, t, v, value));
                }
//                    (*uvtp)[itr->first.first][make_pair(itr->first.second, v)] = itr->second;
            }
        }
/*
        plock->lock();
        for (auto itr = vec.begin(); itr != vec.end(); ++itr)
        {
            p.push_back(*itr);
        }
        plock->unlock();
*/
/*
        int current = -1;
        for (auto itr = uvtp->begin(); itr != uvtp->end(); ++itr)
        {
            int u = itr->first;
            auto pu = itr->second;
            int block = u / BLOCKSIZE;
            if (block > current)
            {
                if (current >= 0)
                    plock[current].unlock();
                plock[current = block].lock();
            }
            if (second[u])
            {
                p[u].insert(pu.lower_bound(make_pair(3, 0)), pu.end());
//                for (auto uitr = pu.lower_bound(make_pair(3, 0)); uitr != pu.end(); ++uitr)
//                    p[u][make_pair(uitr->first.second, uitr->first.first)] = uitr->second;
            }
            else if (first[u])
            {
                p[u].insert(pu.lower_bound(make_pair(2, 0)), pu.end());
//                for (auto uitr = pu.lower_bound(make_pair(2, 0)); uitr != pu.end(); ++uitr)
//                    p[u][make_pair(uitr->first.second, uitr->first.first)] = uitr->second;
            }
            else
            {
                p[u].insert(pu.begin(), pu.end());
//                for (auto uitr = pu.begin(); uitr != pu.end(); ++uitr)
//                    p[u][make_pair(uitr->first.second, uitr->first.first)] = uitr->second;
            }

        }
        if (current >= 0) plock[current].unlock();
*/
//        delete uvtp;
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
//    p.reserve(233976866l);
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

//###################################### Query ################################################
vector<double> Sling::pushback_q(map<pair<int, int>, double, PairCmp>::iterator start,
                                 map<pair<int, int>, double, PairCmp>::iterator end,
                                 double eps, int T) {
    vector<double> p(g->n, 0.);
    vector<double> pp(g->n, 0.);
    deque<int> q;
    deque<int> qq;
    eps = eps * sqrtct[T];
    int TTT = g->n / 10;
    for (; start !=
           end;
           ++start) {
        p[start->first.second] = start->second * d[start->first.second];
        q.push_back(start->first.second);
    }
    for (int i = 0; i < T; ++i) {
        if (q.size() < TTT) {
            while (!q.empty()) {
                int u = q.front();
                q.pop_front();
                for (auto vitr = g->edge[u].begin(); vitr != g->edge[u].end(); ++vitr) {
                    int v = *vitr;
                    double x = pp[v] += sqrtc * p[u] / (double) g->inedge[v].size();
                    if (x > eps && x <= eps + sqrtc * p[u] / (double) g->inedge[v].size()) {
                        qq.push_back(v);
                    }
                }
            }
            p.swap(pp);
            pp.clear();
            pp.resize(g->n, 0.);
            q.swap(qq);
            qq.clear();
        } else {
            for (auto uitr = p.begin(); uitr != p.end(); ++uitr) {
                int u = *uitr;
                if (p[u] < eps) continue;
                for (auto vitr = g->edge[u].begin(); vitr != g->edge[u].end(); ++vitr) {
                    int v = *vitr;
                    pp[v] += sqrtc * p[u] / (double) g->inedge[v].size();
                }
            }
            p.swap(pp);
            pp.clear();
            pp.resize(g->n, 0.);
        }
    }
    return p;
}

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
