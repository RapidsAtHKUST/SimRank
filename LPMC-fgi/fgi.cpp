#include "fgi.h"

void FG_Index::build_index() {
    // sample N trees
    for (int i = 0; i < N; ++i) {
        // init walk length
        len[i] = 1;
        while (rand_gen.double_rand() < c) {
            ++len[i];
        }

        // sample in neighbor
        int selfloop = -1;
        for (int j = 0; j < gn; ++j) {
            f[i][j] = sample_in_neighbor(j, (*g_ptr), rand_gen);
            // leaf[i][f[i][j]] = 0;
            if (f[i][j] >= 0) {
                uf[i].U(j, f[i][j]);
            } else {
                f[i][j] = selfloop;
                selfloop -= len[i]; // avoid conflict between WCCs
            }
            t_pos[i][j] = gn;
        }

        // fill in the t_pos vector
        // TODO: start from leaf node
        for (int j = 0; j < gn; ++j) {
            // if (leaf[i][j] == 0) continue; // what about nodes on cycle?
            if (t_pos[i][j] != gn) continue;
            int p0 = j, p1 = j;
            // len-step ancestor
            for (int k = 0; k < len[i]; ++k) {
                p1 = (p1 < 0) ? (p1 - 1) : f[i][p1];
            }
            while (p0 >= 0 && t_pos[i][p0] == gn) {
                t_pos[i][p0] = p1;
                p0 = f[i][p0];
                p1 = (p1 < 0) ? (p1 - 1) : f[i][p1];
            }
        }
    }

    // for (int i = 0; i < N; ++i) {
    //     cout << "tree #" << i << endl;        
    //     cout << "random walk length " << len[i] << endl;
    //     for (int j = 0; j < gn; ++j) {
    //         cout<<i<<","<<j<<": f "<<f[i][j]<<", t_pos "<<t_pos[i][j]<< endl;
    //     }
    //     cout << endl;
    // }
}

int FG_Index::WCC(const NodePair& np, int i) {
    // cout << t_pos[i][np.first] << " " << t_pos[i][np.second] << " ";
    int x = uf[i].F(np.first);
    int y = uf[i].F(np.second);
    if (x != y) {
        ++fgi_hit;
        return 0;
    }
    // ++fgi_miss;
    return 1;
}

int FG_Index::query(int x, int y, int i) {
    if (t_pos[i][x] == t_pos[i][y]) {
        return 1;
    }
    return 0;
}

int FG_Index::LCA(int x, int y, int i) {
    // if (query(x, y, i)) return -1;
    if (t_pos[i][x] == t_pos[i][y]) {++fgi_hit; return -1;}
    ++fgi_miss;
    for (int p = 0; p < len[i] && y >= 0; ++p) {
        int xx = x;
        for (int q = 0; q < len[i] && xx >= 0; ++q) {
            if (xx == y) {
                return xx;
            }
            xx = f[i][xx];
        };
        y = f[i][y];
    }
    // TODO: skip first/last/non overlap paths
    return gn; 
}


