#ifndef _FINGERPRINT_H_
#define _FINGERPRINT_H_

#include "config.h"

class FingerPrint {
private:
    int *edge;
    int *weights;
    int vnum;

    /* union find set */
    int *parent;
    int *size;
    int *nbrIdx;
public:
    FingerPrint(int vnum);

    ~FingerPrint() {
        delete[] size;
        delete[] edge;
        delete[] weights;
        delete[] parent;
        if (nbrIdx != NULL) {
            delete[] nbrIdx;
        }
    }

    void addEdge(int src, int dest, int w);

    bool isInSameTree(int qv, int otherv);

    void getMeetPoint(int qv, map<int, int> &);

    int calMeetTime(int otherv, map<int, int> &);

    void getCand(int qv, int *cand, int &tail);

    void save(char *filePath);

    void read(char *filePath);

private:
    int find(int qv);

    void merge(int a, int b);
};

#endif /* _FINGERPRINT_H_ */

