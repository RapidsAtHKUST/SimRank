#include "KMSR.h"

KMSR::KMSR(int maxSteps, int sampleNum, double decayfactor, int *graph_src, int *graph_dst, int mvid, char *graph) :
        SimRankMethod(maxSteps, sampleNum, decayfactor), graphSrc(graph_src), graphDst(graph_dst), maxVertexId(mvid) {
    strcpy(graphName, graph);
}

class ComparisonClass {
public:
    bool operator()(SimRankValue v1, SimRankValue v2) {
        return v1.getValue() > v2.getValue();
    }
};

void KMSR::run(int qv, int k) {
    vector<SimRankValue> pq;
    for (int index = 0; index < maxVertexId; ++index) {
        if (qv == index) continue;
        double tmpSimRank = calSimRank(qv, index);
        if (tmpSimRank > 0.0) {
            pq.push_back(SimRankValue(index, tmpSimRank));
        }
    }

    save(pq, k);

    return;

}

double KMSR::calSimRank(int queryv, int otherv) {
    double simrank = 0.0;
    vector<int> queryPath;
    queryPath.resize(sampleNum + 1, 0);
    vector<int> otherPath;
    otherPath.resize(sampleNum + 1, 0);
    vector<int> commonEnds;
    commonEnds.resize(sampleNum + 1, 0);

    int end = 0;
    for (int i = 0; i < sampleNum; ++i) {
        queryPath[i] = queryv;
        otherPath[i] = otherv;
    }

    short exist[maxVertexId];
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
            simrank += factor * 1.0 * ((double) cntq / (double) sampleNum) * ((double) cnto / (double) sampleNum);
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
    return simrank;
}
