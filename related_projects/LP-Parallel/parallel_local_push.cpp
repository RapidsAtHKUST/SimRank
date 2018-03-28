#include "local_push.h"

Parallel_LocalPush::Parallel_LocalPush(DirectedG &g, string name, double c_, double r_max_, size_t n_) :
        LocalPush(g, name, c_, r_max_, n_) {
    cout << "Parallel local push" << endl;
    // init P and R with internal hash maps
    P.add(n);
    R.add(n);
    marker.add(n);
    P_lock = new omp_lock_t[n];
    R_lock = new omp_lock_t[n];
    M_lock = new omp_lock_t[n];
    cout << "init p r complete.." << endl;

    // parallel initilize the 
    double start_time = omp_get_wtime();
    cout << "filling R and Q" << endl;
#pragma omp parallel
    {
#pragma omp for schedule(static)
        for (int i = 0; i < n; i++) {
            NodePair np(i, i);
            R[np] = 1;
            marker[np] = 1;

            // init the locks for P, R and marker
            omp_init_lock(&P_lock[i]);
            omp_init_lock(&R_lock[i]);
            omp_init_lock(&M_lock[i]);
        }

    }

    // init Q and locks
    for (int i = 0; i < n; i++) {
        Q.push(NodePair(i, i));
    }

    double end_time = omp_get_wtime();
    cout << format("initilization uses %ss") % (end_time - start_time) << endl;
    cout << format("Q has %s elements") % Q.size() << endl;
}

void Parallel_LocalPush::local_push(DirectedG &g) {
    // parallel local push method
    // assumption: the elements in Q is unique
    // cout << "begin local push phase" << endl;
    NodePair currentNP;
#pragma omp parallel
    {
#pragma omp single
        {
            while (!Q.empty()) {
                currentNP = Q.front();
#pragma omp task firstprivate(currentNP)
                {
                    // apply for the lock
                    omp_set_lock(&P_lock[currentNP.first]);
                    // do the self-update phase
                    double r = R[currentNP];
                    P[currentNP] += r;
                    R[currentNP] -= r;
                    omp_unset_lock(&P_lock[currentNP.first]);
                }
                Q.pop();
            }
            cout << format("size of Q after push: %s") % Q.size() << endl;
        }
    }
    // cout << "exit local push" << endl;
}
