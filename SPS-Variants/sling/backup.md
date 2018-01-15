### eps 

```cpp
    const static double EPS;
    const static double BACKEPS;
    const static double DEPS;
```

```cpp
    const double Sling::EPS = .025;
    const double Sling::BACKEPS = 7.28e-4; //EPS / 23.;
    const double Sling::DEPS = 5e-3; //EPS * 3. * 3. / 23.;
    const double Sling::K = 10.;
```

### Triangle

```cpp
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

```

### CalD

```cpp
double calcDi_0(int i, double eps, bool &early, int &RWCNT, int tid);
double calcDi_2(int i, double eps, bool &early, int &RWCNT, int tid);
```

```cpp
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
```

```cpp
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
```

### backward

```cpp
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
```

### query

```cpp
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
```