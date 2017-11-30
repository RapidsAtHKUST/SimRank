/*
 * rsamplegraph.hpp
 *
 *  Created on: 2014-7-19
 *      Author: simon0227
 */

#ifndef __RSAMPLEGRAPH_HPP__
#define __RSAMPLEGRAPH_HPP__

/**
 * Store a one-way graph
 */
class RSampleGraph {
public:
    RSampleGraph() : src(NULL), dst(NULL), maxVertexId(0), edgeNum(0) {}

    RSampleGraph(int mid) : maxVertexId(mid), edgeNum(0) {
        src = new int[maxVertexId];
        dst = new int[maxVertexId];
    }

    virtual ~RSampleGraph() {
        delete[] src;
        delete[] dst;
    }

    void preprocess() {
        int *cnt = new int[maxVertexId + 1];
        int *tmp = new int[edgeNum];
        memset(cnt, 0, sizeof(int) * (maxVertexId + 1));

        /* sorting */
        int i;
        int leafNum = 0;
        for (i = 0; i < edgeNum; ++i) {
            //printf("%dth edge, src=%d\n", i, src[i]); 
            ++cnt[src[i]];
        }
        for (i = 1; i <= maxVertexId; ++i) {
            // printf("\tvid=%d, cnt=%d\n", i, cnt[i]);
            if (cnt[i] == 0) leafNum++;
            cnt[i] += cnt[i - 1];
        }/* NOTE: this is end point*/
        if (cnt[0] == 0) leafNum++;
        //     printf("\tvid=%d, cnt=%d\n", 0, cnt[0]);
        for (i = 0; i < edgeNum; ++i) {
            --cnt[src[i]];
            tmp[cnt[src[i]]] = dst[i];
        }

        //printf("LeafNum=%d\n", leafNum);

        delete[] src;
        src = cnt; /* index for the sorted edge */
        delete[] dst;
        dst = tmp; /* sorted edge */
    }

    /* the new edge has been reversed before insertion. */
    void addEdge(int a, int b) {
        src[edgeNum] = a;
        dst[edgeNum] = b;
        edgeNum++;
        //     printf("edgeNum=%d, (%d, %d)\n", edgeNum, a, b);
    }

    void clear() {
        delete[] dst;
        dst = new int[maxVertexId];
        edgeNum = 0;
    }

    void expand(int vid, queue<int> &q) {
        int h = src[vid], t = src[vid + 1];
        while (h < t) {
            q.push(dst[h]);
            ++h;
        }
    }

    /* xx MB of the memory cost. */
    double getMemSize() {
        return (sizeof(int) * (maxVertexId + edgeNum + 2)) / 1024.0 / 1024.0;
    }

protected:
    int *src;
    int *dst;
    int maxVertexId;
    int edgeNum;
};

#endif /* RSAMPLEGRAPH_HPP_ */
