#include "KMSRIndex.h"

KMSRIndex::KMSRIndex(int maxSteps, int sampleNum, double decayfactor, int *graph_src, int *graph_dst, int mvid,
                     char *graph, bool isinit, int *ori_graph_src, int *ori_graph_dst) :
        SimRankMethod(maxSteps, sampleNum, decayfactor), graphSrc(graph_src), graphDst(graph_dst), maxVertexId(mvid),
        isInit(isinit), oriGraphSrc(ori_graph_src), oriGraphDst(ori_graph_dst) {
    strcpy(graphName, graph);
    sampleBipartite = NULL;
    P = 10;
    T = 10;
    Q = 5;
    R = 100;
    R1 = 10;
    R2 = 100;
    epsilon = 0.00001;
    ga_mma = new double *[T];
    for (int i = 0; i < T; ++i) {
        ga_mma[i] = new double[mvid];
    }
}

class ComparisonClass {
public:
    bool operator()(SimRankValue v1, SimRankValue v2) {
        return v1.getValue() > v2.getValue();
    }
};

void KMSRIndex::initialize() {
    Time timer;
    timer.start();
    if (isInit == false) {
        preprocess(); /* compute bipartite graph and gamma */
        timer.stop();
        char sgpath[125];
        sprintf(sgpath, "dataset/%s/index/kmsr/kmsr.index", graphName);
        FILE *fp = fopen(sgpath, "wb");
        /* save bipartite graph */
        fwrite(&maxVertexId, sizeof(int), 1, fp);
        fwrite(&T, sizeof(int), 1, fp);
        for (int i = 0; i < maxVertexId; i++) {
            fwrite(sampleBipartite[i], sizeof(int), sampleBipartite[i][0] + 1, fp);
        }
        /* save gamma */
        for (int i = 0; i < T; ++i) {
            fwrite(ga_mma[i], sizeof(double), maxVertexId, fp);
        }
        fclose(fp);
        isInit = true;
    } else if (sampleBipartite == NULL) {
        /* read from file. */
        char sgpath[125];
        sprintf(sgpath, "dataset/%s/index/kmsr/kmsr.index", graphName);
        FILE *fp = fopen(sgpath, "rb");
        fread(&maxVertexId, sizeof(int), 1, fp);
        fread(&T, sizeof(int), 1, fp);
        sampleBipartite = new int *[maxVertexId];
        for (int i = 0; i < maxVertexId; ++i) {
            int tmp;
            fread(&tmp, sizeof(int), 1, fp);
            sampleBipartite[i] = new int[tmp + 1];
            sampleBipartite[i][0] = tmp;
            fread(sampleBipartite[i] + 1, sizeof(int), tmp, fp);
        }
        for (int i = 0; i < T; ++i) {
            fread(ga_mma[i], sizeof(double), maxVertexId, fp);
        }
        fclose(fp);
    }
    timer.stop();
    //printf("Building index cost: %.5lf.\n", timer.getElapsedTime());
}

int KMSRIndex::performRandomWalk(int vertex, int stepLen, int *route) {
    int routeLength = 0;
    int minIndexOfNeighbor = graphSrc[vertex], maxIndexOfNeighbor = graphSrc[vertex + 1];
    if (minIndexOfNeighbor >= maxIndexOfNeighbor) {
        return routeLength;
    }
    int bias = maxIndexOfNeighbor - minIndexOfNeighbor;
    route[0] = (graphDst[minIndexOfNeighbor + randomGenerator.getRandom() % bias]);
    for (routeLength = 1; routeLength < stepLen; ++routeLength) {
        minIndexOfNeighbor = graphSrc[route[routeLength - 1]];
        maxIndexOfNeighbor = graphSrc[route[routeLength - 1] + 1];
        if (minIndexOfNeighbor >= maxIndexOfNeighbor) {
            return routeLength;
        }
        bias = maxIndexOfNeighbor - minIndexOfNeighbor;
        route[routeLength] = (graphDst[minIndexOfNeighbor + randomGenerator.getRandom() % bias]);
    }
    return routeLength;
}

bool KMSRIndex::preprocess() {
    sampleBipartite = new int *[maxVertexId];
    if (sampleBipartite == NULL) {
        return false;
    }
    memset(sampleBipartite, 0, sizeof(int *) * maxVertexId);

    int *randomWalk = new int[T];
    int **sampleRandomWalk = new int *[Q];
    for (int i = 0; i < Q; ++i) {
        sampleRandomWalk[i] = new int[T];
    }

    int *isIn = new int[maxVertexId];
    int ts = 0;
    memset(isIn, -1, sizeof(int) * maxVertexId);

    for (int i = 0; i < maxVertexId; ++i) {
        vector<int> neighbor;
        ts++;

        for (int j = 0; j < P; ++j) {
            int res = performRandomWalk(i, T, randomWalk);
            if (res <= 0) { continue; }
            for (int ll = 0; ll < Q; ++ll) {
                memset(sampleRandomWalk[ll], -1, sizeof(int) * res);
                performRandomWalk(i, res, sampleRandomWalk[ll]);
            }
            for (int t = 0; t < res; ++t) {
                int numOfFrequentNeighbor = 0;
                for (int ll = 0; ll < Q; ++ll) {
                    if (sampleRandomWalk[ll][t] == randomWalk[t])
                        numOfFrequentNeighbor++;
                }
                if (numOfFrequentNeighbor >= 2 && isIn[randomWalk[t]] < ts) {
                    neighbor.push_back(randomWalk[t]);
                    isIn[randomWalk[t]] = ts;
                } // end if
            } // end for
        } //end for

        /* add edges in the bipartite graph. */
        unsigned int neighborNum = neighbor.size();
        sort(neighbor.begin(), neighbor.end());
        sampleBipartite[i] = new int[neighborNum + 1];
        if (sampleBipartite[i] == NULL) {
            return false;
        }
        sampleBipartite[i][0] = neighborNum;
        for (unsigned k = 0; k < neighborNum; ++k) {
            sampleBipartite[i][k + 1] = neighbor[k];
        }
    } // end for
    delete[] randomWalk;
    for (int i = 0; i < Q; ++i) {
        delete[] sampleRandomWalk[i];
    }
    delete[] sampleRandomWalk;
    delete isIn;

    /* compute gamma */
    computeGamma();
//	precomputeBeta( );
    return true;
} // end procedure*/

int KMSRIndex::findIntersection(int qv, int otherv) {
    int qIdx = 1, oIdx = 1;
    while (qIdx <= sampleBipartite[qv][0] && oIdx <= sampleBipartite[otherv][0]) {
        int qnb = sampleBipartite[qv][qIdx];
        int onb = sampleBipartite[otherv][oIdx];
        if (qnb == onb) return 1;
        if (qnb < onb) qIdx++;
        else oIdx++;
    }
    return 0;
}

void KMSRIndex::computeGamma() {
    for (int i = 0; i < maxVertexId; ++i) {
        vector<int> neighbor;
        neighbor.resize(R, i);
        for (int d = 0; d < T; ++d) {
            double tmp = 0.0;

            sort(neighbor.begin(), neighbor.end(), greater<int>());
            vector<int> uniqueNeighbor;
            uniqueNeighbor.resize(R);
            vector<int>::iterator uniqueBound = unique_copy(neighbor.begin(), neighbor.end(), uniqueNeighbor.begin());

            int sid = 0;
            int nsize = neighbor.size();
            for (vector<int>::iterator iter = uniqueNeighbor.begin(); iter != uniqueBound; ++iter) {
                if (*iter == -1) break;
                int cnt = 0;
                /* neighbor is sorted. */
                while (sid < nsize && neighbor[sid] == (*iter)) {
                    sid++;
                    cnt++;
                }
                //D = 1.0 - decayFactor in the paper.
                //count(neighbor.begin(),neighbor.end(),*iter)
                tmp += (1.0 - decayFactor) * pow((1.0 * cnt) / R, 2);
            }

            ga_mma[d][i] = sqrt(tmp);
            for (int k = 0; k < R; ++k) {
                int tmpNeighbor = neighbor[k];
                if (tmpNeighbor == -1) break;
                if (graphSrc[tmpNeighbor] != graphSrc[tmpNeighbor + 1]) {
                    int index = randomGenerator.getRandom() % (graphSrc[tmpNeighbor + 1] - graphSrc[tmpNeighbor]);
                    neighbor[k] = graphDst[graphSrc[tmpNeighbor] + index];
                } else {
                    neighbor[k] = -1;
                }
            }
        }
    }
}

double KMSRIndex::computeBeta(int vertex, int d, int *predist) {
    vector<int> neighbor;
    neighbor.resize(R, vertex);
    double **ALPHA;
    ALPHA = new double *[T];
    for (int i = 0; i < T; ++i) {
        ALPHA[i] = new double[T + 1];
        memset(ALPHA[i], 0, sizeof(double) * (T + 1));
    }
    for (int t = 0; t < T; ++t) {
        sort(neighbor.begin(), neighbor.end(), greater<int>());
        vector<int> uniqueNeighbor;
        uniqueNeighbor.resize(R);
        vector<int>::iterator uniqueBound = unique_copy(neighbor.begin(), neighbor.end(), uniqueNeighbor.begin());
        vector<int>::iterator iter = uniqueNeighbor.begin();
        while (iter != uniqueBound) {
            if ((*iter) == -1) break;
            int dist = predist[*iter]; //BFS(u,*iter);
            double tmpmax = ALPHA[t][dist];
            ALPHA[t][dist] = max(tmpmax, (1.0 - decayFactor) * count(neighbor.begin(), neighbor.end(), *iter) * 1.0 /
                                         (1.0 * R));
            ++iter;
        }
        for (int i = 0; i < R; ++i) {
            int tmpNeighbor = neighbor[i];
            if (tmpNeighbor == -1) break;
            if (graphSrc[tmpNeighbor] != graphSrc[tmpNeighbor + 1]) {
                neighbor[i] = graphDst[graphSrc[tmpNeighbor] + randomGenerator.getRandom() %
                                                               (graphSrc[tmpNeighbor + 1] - graphSrc[tmpNeighbor])];
            } else {
                neighbor[i] = -1;
            }
        }
    }

    double tmpsum = 0.0;
    double factor = 1.0;
    for (int i = 1; i < T; ++i) {
        double tmpmax = -1.0;
        int leftDist = (d - i >= 0 ? d - i : 0);
        int rightDist = (d + i < T ? d + i : T);
        for (int j = leftDist; j <= rightDist; ++j) {
            tmpmax = max(tmpmax, ALPHA[i][j]);
        }
        tmpsum += factor * tmpmax;
        factor *= decayFactor;
    }

    return tmpsum;
}

void KMSRIndex::run(int qv, int k) {
    priority_queue<SimRankValue, vector<SimRankValue>, ComparisonClass> pq;

    /*1. initialization: building bipartite graph and compute gama bound. */
    initialize();

    /*2. prune by L1 and L2 bound */
    Time timer;
    timer.start();

    /* compute distance */
    int *dist = new int[maxVertexId];
    memset(dist, -1, sizeof(int) * maxVertexId);
    bool *vis = new bool[maxVertexId];
    memset(vis, 0, sizeof(bool) * maxVertexId);
    compDist(qv, dist, vis);
    timer.stop();

    timer.reset();
    timer.start();
    int curV = qv;
    int maxDist = 10;//set 3 for large graphs
    queue<int> Que;
    int tmpDist;
    int pred = -1;
    memset(vis, 0, sizeof(bool) * maxVertexId);

    vis[curV] = true;
    Que.push(curV);
    int filter = 0, fb = 0, fg = 0, tot = 0;
    while (Que.empty() == false) {
        tot++;
        curV = Que.front();
        Que.pop();
        tmpDist = dist[curV];
        //1. compute beta
        if (pred != tmpDist) {
            double betaBound = computeBeta(qv, tmpDist, dist);
            if ((int) pq.size() >= k && betaBound < pq.top().getValue()) {
                fb++;
                continue;
            }  //end if
            pred = tmpDist;
        }
        if (tmpDist < maxDist) {
            expandQueueOnUndirected(curV, Que, vis);
        }
        /*prune and compute here!*/
        //2. prune by H, too harsh.....
//		if(findIntersection(qv, curV) == 0) { filter++; continue;}

        //3. prune gamma
        if (pq.size() >= (unsigned int) k) {
            double factor = 1.0;
            double gammaBound = 0.0;
            /* maybe here have no effectiveness. */
            for (int t = 1; t < T; ++t, factor *= decayFactor) {
                gammaBound += factor * ga_mma[t][qv] * ga_mma[t][curV];
            }
            if (gammaBound < pq.top().getValue()) {
                fg++;
                continue;
            } //pruning
        }
        if (qv != curV) {
            double srvalue = calSimRank(qv, curV);
            pq.push(SimRankValue(curV, srvalue));
            if (pq.size() > (unsigned int) k) {
                pq.pop();
            }
        }
    }

    timer.stop();
    delete[] dist;
    delete[] vis;

    vector<SimRankValue> res;
    while (pq.empty() == false) {
        const SimRankValue &srp = pq.top();
        res.push_back(SimRankValue(srp.getVid(), srp.getValue()));
        pq.pop();
    }
    save(res, k);
    return;

}

void KMSRIndex::expandQueueOnUndirected(int vertex, queue<int> &Que, bool *vis, int *dist) {
    int minIndex = graphSrc[vertex];
    int maxIndex = graphSrc[vertex + 1];
    for (int k = minIndex; k < maxIndex; ++k) {
        if (vis[graphDst[k]] == false) {
            vis[graphDst[k]] = true;
            if (dist != NULL) {
                dist[graphDst[k]] = dist[vertex] + 1;
            }
            Que.push(graphDst[k]);
        }
    }
    minIndex = oriGraphSrc[vertex];
    maxIndex = oriGraphSrc[vertex + 1];
    for (int k = minIndex; k < maxIndex; ++k) {
        if (vis[oriGraphDst[k]] == false) {
            vis[oriGraphDst[k]] = true;
            if (dist != NULL) {
                dist[oriGraphDst[k]] = dist[vertex] + 1;
            }
            Que.push(oriGraphDst[k]);
        }
    }
}

void KMSRIndex::compDist(int vertex, int *dist, bool *vis) {
    int curV = vertex;
    queue<int> Que;
    Que.push(curV);
    vis[curV] = true;
    dist[curV] = 0;
    while (Que.empty() == false) {
        curV = Que.front();
        Que.pop();
        expandQueueOnUndirected(curV, Que, vis, dist);
    }
}

double KMSRIndex::calSimRank(int queryv, int otherv) {
    double simrank = 0.0;
    vector<int> queryPath;
    queryPath.resize(sampleNum + 1, 0);
    vector<int> otherPath;
    otherPath.resize(sampleNum + 1, 0);

    int end = 0;
    for (int i = 0; i < sampleNum; ++i) {
        queryPath[i] = queryv;
        otherPath[i] = otherv;
    }

    short *exist = new short[maxVertexId];
    int vt = 0;
    memset(exist, 0, sizeof(short) * maxVertexId);

    vector<int> qEnd;
    vector<int> oEnd;
    for (int i = 1; i <= maxSteps; ++i) {
        end = 0;
        vt++;
        double factor = pow(decayFactor, i);
        oEnd.clear();
        qEnd.clear();
        for (int x = 0; x < sampleNum; ++x) {
            if (queryPath[x] != -1) {
                int size_queryvertex = graphSrc[queryPath[x] + 1] - graphSrc[queryPath[x]];
                if (size_queryvertex == 0) {
                    queryPath[x] = -1;
                    continue;
                }
                int randomIndex = abs(randomGenerator.getRandom()) % size_queryvertex;
                queryPath[x] = graphDst[graphSrc[queryPath[x]] + randomIndex];
                qEnd.push_back(queryPath[x]);
            }

            if (otherPath[x] != -1) {
                int size_othervertex = graphSrc[otherPath[x] + 1] - graphSrc[otherPath[x]];
                if (size_othervertex == 0) {
                    otherPath[x] = -1;
                    continue;
                }
                int randomIndex = abs(randomGenerator.getRandom()) % size_othervertex;
                //               if(randomIndex < 0) printf("error!");
                otherPath[x] = graphDst[graphSrc[otherPath[x]] + randomIndex];
                oEnd.push_back(otherPath[x]);
            }

        }
        if (qEnd.size() == 0 || oEnd.size() == 0) break;
        sort(qEnd.begin(), qEnd.end());
        sort(oEnd.begin(), oEnd.end());
        unsigned int opt = 0, qpt = 0;
        int cntq = 0, cnto = 0;//
        while (qpt < qEnd.size()) {
            cntq = 1;
            while (qpt + 1 < qEnd.size() && qEnd[qpt] == qEnd[qpt + 1]) {
                ++qpt;
                cntq++;
            }
            ++qpt;
            cnto = 0;
            while (opt < oEnd.size()) {
                if (oEnd[opt] > qEnd[qpt - 1]) break;
                if (oEnd[opt] == qEnd[qpt - 1]) {
                    cnto = 1;
                    while (opt + 1 < oEnd.size() && oEnd[opt] == oEnd[opt + 1]) {
                        ++opt;
                        cnto++;
                    }
                    ++opt;
                } else {
                    opt++;
                }
            }
            //    printf("qpt=%d opt=%d\n", qpt, opt);
            //use the (1-c)I initializations.
            simrank += factor * (1.0 - decayFactor) * ((double) cntq / (double) sampleNum) *
                       ((double) cnto / (double) sampleNum);
        }
/*		
		for(int x = 0; x < sampleNum; ++x){
			if(queryPath[x] == -1 || exist[queryPath[x]] == vt) { continue;}
			for(int y = 0; y < sampleNum; ++y){
				if(queryPath[x] == otherPath[y]){
					commonEnds[end] = queryPath[x];
					exist[queryPath[x]] = vt;
					++end;
					break;
				}
			}
		}
		
		for(int z = 0; z < end; ++z){
			int cntq = 0;
			int cnto = 0;
			for(int x = 0; x < sampleNum; ++x){
				if(queryPath[x] == commonEnds[z]) { ++cntq; }
				if(otherPath[x] == commonEnds[z]) { ++cnto; }
			}
//            if(cntq > 0 && cnto > 0){
//                printf("(qv=%d, ov=%d, z=%d): cntq=%d cnto=%d\n", queryv, otherv, z, cntq, cnto);
//            }
			simrank += factor * 1.0 * ((double)cntq/(double)sampleNum) * ((double)cnto/(double)sampleNum);
		}
        */
    }
    delete[] exist;
    return simrank;
}
