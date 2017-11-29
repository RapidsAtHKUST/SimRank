#include "reads.h"
#include <math.h>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstring>
// #include "inBuf.h"
// #include "outBuf.h"
// #include "meminfo.h"
#include "sparsehash/dense_hash_map"

using google::dense_hash_map;
using namespace std;

reads::reads(char *gName_, int n_, int r_, double c_, int t_) {
    sprintf(gName, "%s", gName_);
    n = n_, r = r_, c = c_, t = t_;
    t1 = t2 = qCnt = 0;

    char iName[125];
    sprintf(iName, "%s.reads.%d_%d_%lf_%d", gName, n, r, c, t);

    // if (fopen(iName, "rb") != NULL)
    // {
    // 	printf("load ");
    // 	inBuf buf(iName);
    // 	ef.resize(n);
    // 	eb.resize(n);
    // 	nt.resize(r);
    // 	for (int i = 0; i < r; i++)
    // 	{
    // 		nt[i].resize(n);
    // 		for (int j = 0; j < n; j++)
    // 			buf.nextInt(nt[i][j]);
    // 	}

    // 	for (int i = 0, s; i < n; i++)
    // 	{
    // 		buf.nextInt(s);
    // 		ef[i].resize(s);
    // 		for (int j = 0; j < s; j++)
    // 			buf.nextInt(ef[i][j]);
    // 	}
    // 	for (int i = 0, s; i < n; i++)
    // 	{
    // 		buf.nextInt(s);
    // 		eb[i].resize(s);
    // 		for (int j = 0; j < s; j++)
    // 			buf.nextInt(eb[i][j]);
    // 	}
    // 	rtime = 0;
    // 	return;
    // }

    FILE *fg = fopen(gName, "r");
    if (fg == NULL) {
        printf("No graph %s\n", gName);
        exit(0);
    }

    Timer tm;
    ef.resize(n);
    eb.resize(n);

    for (int x, y; fscanf(fg, "%d%d", &x, &y) != EOF;) {
        ef[x].push_back(y);
        eb[y].push_back(x);
    }

    rtime = tm.getTime();



//printf("loaded graph\n");
    fclose(fg);

    int q0, q1, q2;
    vector<pair<int, int> > q;
    vector<int> pos;
    int cc = int(RAND_MAX * sqrt(c));
    vector<int> sta, tmp(n);
    int tmpCnt;


    nt.resize(r);
    for (int i = 0; i < r; i++)
        nt[i].resize(n);
//printf("init\n");

    for (int i = 0; i < r; i++) {
//printf("%d\n", i);

        q.resize(0);
        pos.assign(n, -1);

        for (int j = 0, p; j < n; j++)
            if (!eb[j].empty()) {
                if (pos[p = eb[j][rand() % eb[j].size()]] < 0) {
                    pos[p] = q.size();
                    q.push_back(make_pair(p, -1));
                }
                nt[i][j] = pos[p];
//printf("%d ", p);
            } else nt[i][j] = -1;
//printf("\n");


        q0 = 0;
        q1 = q.size();
        for (int j = 0, p, rr; j < t - 1 && q0 < q1; j++) {
            int tt;
            for (; q0 < q1; q0++) {
                if (!eb[q[q0].first].empty() && ((rr = rand()) < cc || j == 0)) {
                    p = eb[q[q0].first][rr % eb[q[q0].first].size()];
                    if (pos[p] < q1) {
                        pos[p] = q.size();
                        q.push_back(make_pair(p, -1));
                    }
                    q[q0].second = pos[p];
                }
//printf("%d ", rr);
            }
            q1 = q.size();
        }
//printf("\n");

        tmpCnt = 0;
        for (int j = 0, qid; j < n; j++)
            if (nt[i][j] > -1) {
                for (sta.resize(1), sta[0] = nt[i][j]; q[*sta.rbegin()].second >= 0;)
                    sta.push_back(q[*sta.rbegin()].second);

                if ((qid = -q[*sta.rbegin()].second - 2) < 0) {
                    q[*sta.rbegin()].second = -tmpCnt - 2;
                    tmp[tmpCnt] = pos[tmpCnt] = j;
                    tmpCnt++;
                } else {
                    nt[i][tmp[qid]] = j;
                    tmp[qid] = j;
                }
                for (int k = sta.size() - 2; k >= 0; k--)
                    q[sta[k]].second = q[*sta.rbegin()].second;
            }

        for (int j = 0; j < tmpCnt; j++)
            nt[i][tmp[j]] = pos[j];

    }

    for (int i = 0; i < n; i++) {
        random_shuffle(ef[i].begin(), ef[i].end());
        random_shuffle(eb[i].begin(), eb[i].end());
    }

    return;


    // tm.reset();
    // outBuf buf(iName);

    // for (int i = 0; i < r; i++)
    // 	for (int j = 0; j < n; j++)
    // 		buf.insert(nt[i][j]);

    // for (int i = 0; i < n; i++)
    // {
    // 	buf.insert(ef[i].size());
    // 	for (auto j = ef[i].begin(); j != ef[i].end(); j++)
    // 		buf.insert(*j);
    // }

    // for (int i = 0; i < n; i++)
    // {
    // 	buf.insert(eb[i].size());
    // 	for (auto j = eb[i].begin(); j != eb[i].end(); j++)
    // 		buf.insert(*j);
    // }
    // rtime += tm.getTime();

}


reads::~reads() {
}

double reads::queryOne(int x, int y) {
}

void reads::queryAll(int x, double *ansVal) {

    memset(ansVal, 0, sizeof(double) * n);
    if (eb[x].empty()) {
        ansVal[x] = 1;
        return;
    }

    if (qCnt >= 20 && t1 < t2 || qCnt < 20 && qCnt % 2 == 0) {
        if (qCnt < 20) tm.reset();

        dense_hash_map<int, double> sim1;
        sim1.set_empty_key(-1);
        // double * sim1 = new double[n];
        // memset(sim1, 0, sizeof(double)*n);

        // double * sim2 = new double[n];
        // memset(sim2, 0, sizeof(double)*n);

        //sim1, sim2, ansVal = real*g[x][0]*r/c^2


        bool *nID = new bool[n];
        memset(nID, 0, sizeof(bool) * n);

        double cc = (1 - c) * r, c1 = r / c;

        // for (int i = 1; i <= g[x][0]; i++)
        // 	for (int j = 1, p = g[x][i]; j <= g[p][0]; j++)
        // 		sim2[g[p][j]] += cc/g[p][0];

        // for (int i = 0; i < n; i++)
        // 	if (sim2[i] > 0)
        // 	for (int j = 1; j <= f[i][0]; j++)
        // 		sim1[f[i][j]] += sim2[i]/g[f[i][j]][0];


        //	if (0)

        int p, q;
        //	if (0)
        for (auto i = eb[x].begin(); i != eb[x].end(); i++) {
            for (auto j = eb[p = *i].begin(); j != eb[p].end() && j - eb[p].begin() < 10; j++)
                for (auto k = ef[q = *j].begin(); k != ef[q].end() && k - ef[q].begin() < 10; k++)
                    if (*k != p)
                        sim1[*k] += cc / eb[p].size() / eb[*k].size();
            sim1[*i] += c1;
        }




        //	if (0)
        for (int i = 0, cnt, q; i < r; i++) {
            for (auto j = eb[x].begin(); j != eb[x].end(); j++)
                nID[*j] = 1;

            for (auto j = eb[x].begin(); j != eb[x].end(); j++)
                if (nID[q = *j] && nt[i][q] != q && nt[i][q] != -1) {
                    cnt = 0;

                    for (int p = nt[i][q]; p != q; p = nt[i][p]) {
                        if (nID[p]) cnt++, nID[p] = 0;
                        else sim1[p] += c;
                        //					nID[p] = 0;
                    }

                    if (cnt != 0) {
                        cc = c * cnt;
                        for (int p = nt[i][q]; p != q; p = nt[i][p])
                            sim1[p] += cc;
                        sim1[q] += cc;
                    }
                }

        }






        // for (int i = 0, j, cnt, *leafPos, nSize; i < r; i++)
        // {
        // 	nSize = 0;
        // 	for (j = 1; j <= g[x][0]; j++)
        // 		if (leaf[i][g[x][j]] > -1) nBeg[nSize++] = leaf[i][g[x][j]];

        // 	sort(nBeg, nBeg + nSize);

        // 	nBeg[nSize] = -1;

        // 	for (j = 1, cnt = 1; j <= nSize; j++)
        // 		if (nBeg[j] == nBeg[j-1]) cnt++;
        // 		else
        // 		{
        // 			for	(leafPos = leaf[i] + nBeg[j-1]; *leafPos >= 0; leafPos++)
        // 				if (nID[*leafPos])
        // 					sim1[*leafPos] += c*(cnt-1);// - (1-c)/g[*leafPos][0];
        // 				else
        // 					sim1[*leafPos] += c*cnt;
        // 			if (nID[*leafPos + n])
        // 				sim1[*leafPos + n] += c*(cnt-1);// - (1-c)/g[*leafPos+n][0];
        // 			else
        // 				sim1[*leafPos + n] += c*cnt;
        // 			cnt = 1;
        // 		}
        // }



        cc = c * c / eb[x].size() / r;
        for (auto it = sim1.begin(); it != sim1.end(); it++)
            for (auto jt = ef[p = it->first].begin(); jt != ef[p].end(); jt++)
                ansVal[*jt] += it->second * cc / eb[*jt].size();

        // if (0)
        // for (int i = 0; i < n; i++)
        // 	if (sim1[i] > 0)
        // 	for (auto j = ef[i].begin(); j != ef[i].end(); j++)
        // 		ansVal[*j] += sim1[i];


        //	for (int i = 0; i < n; i++)	printf("%lf %d\n", ansVal[i], i);

        // for (int i = 0; i < n; i++)
        // 	if (!eb[i].empty())
        // 	ansVal[i] = ansVal[i] / eb[x].size() / eb[i].size();

        //	delete [] sim1;
        delete[] nID;

        if (qCnt++ < 20) t1 += tm.getTime();

    } else {

        if (qCnt < 20) tm.reset();

        double *sim1 = new double[n];
        memset(sim1, 0, sizeof(double) * n);

        // double * sim2 = new double[n];
        // memset(sim2, 0, sizeof(double)*n);

        //sim1, sim2, ansVal = real*g[x][0]*r/c^2


        bool *nID = new bool[n];
        memset(nID, 0, sizeof(bool) * n);

        double cc = (1 - c) * r, c1 = r / c;

        // for (int i = 1; i <= g[x][0]; i++)
        // 	for (int j = 1, p = g[x][i]; j <= g[p][0]; j++)
        // 		sim2[g[p][j]] += cc/g[p][0];

        // for (int i = 0; i < n; i++)
        // 	if (sim2[i] > 0)
        // 	for (int j = 1; j <= f[i][0]; j++)
        // 		sim1[f[i][j]] += sim2[i]/g[f[i][j]][0];


        //	if (0)

        int p, q;
        //	if (0)
        for (auto i = eb[x].begin(); i != eb[x].end(); i++) {
            for (auto j = eb[p = *i].begin(); j != eb[p].end() && j - eb[p].begin() < 10; j++)
                for (auto k = ef[q = *j].begin(); k != ef[q].end() && k - ef[q].begin() < 10; k++)
                    if (*k != p)
                        sim1[*k] += cc / eb[p].size() / eb[*k].size();
            sim1[*i] += c1;
        }





        //	if (0)
        for (int i = 0, cnt, q; i < r; i++) {
            for (auto j = eb[x].begin(); j != eb[x].end(); j++)
                nID[*j] = 1;

            for (auto j = eb[x].begin(); j != eb[x].end(); j++)
                if (nID[q = *j] && nt[i][q] != q && nt[i][q] != -1) {
                    cnt = 0;

                    for (int p = nt[i][q]; p != q; p = nt[i][p]) {
                        if (nID[p]) cnt++, nID[p] = 0;
                        else sim1[p] += c;
                        //					nID[p] = 0;
                    }

                    if (cnt != 0) {
                        cc = c * cnt;
                        for (int p = nt[i][q]; p != q; p = nt[i][p])
                            sim1[p] += cc;
                        sim1[q] += cc;
                    }
                }

        }






        // for (int i = 0, j, cnt, *leafPos, nSize; i < r; i++)
        // {
        // 	nSize = 0;
        // 	for (j = 1; j <= g[x][0]; j++)
        // 		if (leaf[i][g[x][j]] > -1) nBeg[nSize++] = leaf[i][g[x][j]];

        // 	sort(nBeg, nBeg + nSize);

        // 	nBeg[nSize] = -1;

        // 	for (j = 1, cnt = 1; j <= nSize; j++)
        // 		if (nBeg[j] == nBeg[j-1]) cnt++;
        // 		else
        // 		{
        // 			for	(leafPos = leaf[i] + nBeg[j-1]; *leafPos >= 0; leafPos++)
        // 				if (nID[*leafPos])
        // 					sim1[*leafPos] += c*(cnt-1);// - (1-c)/g[*leafPos][0];
        // 				else
        // 					sim1[*leafPos] += c*cnt;
        // 			if (nID[*leafPos + n])
        // 				sim1[*leafPos + n] += c*(cnt-1);// - (1-c)/g[*leafPos+n][0];
        // 			else
        // 				sim1[*leafPos + n] += c*cnt;
        // 			cnt = 1;
        // 		}
        // }



        // q = eb[x].size();
        // for (auto it = sim1.begin(); it != sim1.end(); it++)
        // 	for (auto jt = ef[p=it->first].begin(); jt != ef[p].end(); jt++)
        // 		ansVal[*jt] += it->second/eb[*jt].size()/q;

        for (int i = 0; i < n; i++)
            if (sim1[i] > 0)
                for (auto j = ef[i].begin(); j != ef[i].end(); j++)
                    ansVal[*j] += sim1[i];

        cc = c * c / r / eb[x].size();
        for (int i = 0; i < n; i++)
            if (!eb[i].empty())
                ansVal[i] = ansVal[i] * cc / eb[i].size();

        //	delete [] sim1;
        delete[] nID;

        if (qCnt++ < 20) t2 += tm.getTime();
    }
}




