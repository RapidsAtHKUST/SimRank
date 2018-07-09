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
            uf[i].U(j, f[i][j]);
            if (f[i][j] < 0) {
                f[i][j] = selfloop;
                selfloop -= len[i]; // avoid conflict between WCCs
            }
            t_pos[i][j] = gn;
        }

        // fill in the t_pos vector
        // TODO: start from leaf node
        for (int j = 0; j < gn; ++j) {
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

int FG_Index::query(const NodePair& np, int i) {
    // cout << t_pos[i][np.first] << " " << t_pos[i][np.second] << " ";
    if (uf[i].F(np.first) != uf[i].F(np.second)) {
        ++fgi_hit;
        return 0;
    }
    ++fgi_miss;
    return -1;
    /*
    if (t_pos[i][np.first] == t_pos[i][np.second]) {
        ++fgi_hit;
        return 1;
    }
    ++fgi_miss;
    return -1;
    */
}
