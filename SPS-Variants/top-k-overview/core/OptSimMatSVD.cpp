/*************************************************************************
    > File Name: EffiEVD.cpp
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Sun 22 Mar 2015 02:37:07 PM CST
 ************************************************************************/
#include "OptSimMatSVD.h"

void OptSimMatSVD::run(int qv, int k) {
    if (isInit == false) {
        isInit = true;
        firstRun = false;
        initialize();
    } else {
        //read from path
        if (firstRun == true) {
            L.load(Lpath);
            R.load(Rpath);
            l_.load(l_path);
            r_.load(r_path);
            l_dev.load(l_devpath);
            r_dev.load(r_devpath);

            FILE *fp = fopen(indexpath, "rb");
            if (fp == NULL) {
                printf("Failed to open the %s file.\n", indexpath);
            }

            fread(&index[0], sizeof(int), maxVertexId, fp);
            fclose(fp);
            FILE *fpRev = fopen(revIndexpath, "rb");
            if (fp == NULL) {
                printf("Failed to open the %s file.\n", revIndexpath);
            }

            fread(&revIndex[0], sizeof(int), maxVertexId, fpRev);
            fclose(fpRev);
        }

    }
    //normal topk alg
    double score = 0.0;
    vector<SimRankValue> res;
    res.resize(maxVertexId);
    for (int i = 0; i < maxVertexId; i++) {
        score = getScore(qv, i);
        res[i].setVid(revIndex[i]);
        res[i].setValue(score);
    }
    save(res, k);

    //method in paper
    /*
    vector<SimRankValue> res;
        res.resize(k);
        double theta=0;
        //priority_queue<pair<double,int>,vector<pair<double,int> >,greater<pair<double,int> > >coll;
        for(int i=0;i<k;i++){
        coll.push(make_pair(0,i));
        }
        cout<<"we have added k dummy nodes here"<<endl;
        double Su_qi,S_qi;
        for(int i=0;i<maxVertexId;i++){
            if(i==qv) continue;
            Su_qi=getSu(qv,i);
            if(Su_qi>=theta){
                S_qi=getScore(qv,i);
                // cout<<"we got S_qi"<<S_qi<<endl;
                if(S_qi>theta){
                    coll.pop();
                    coll.push(make_pair(S_qi,i));
                    theta=coll.top().first;
                }
            }
        }

        int count=0;
        while(!coll.empty()){
            cout<<"node:"<<coll.top().first<<"      score:"<<coll.top().second<<endl;
            res[count].setVid(index[coll.top().second]);
            res[count].setValue(coll.top().first);
            count++;
            printf("count is %d\n",count);
            coll.pop();
        }
        results.clear();
        for(int i=k-1;i>=0;i--){
            results.push_back(res[i]);
        }
         */
}

void OptSimMatSVD::initialize() {
    mat W_t(maxVertexId, maxVertexId, fill::zeros);
    for (int i = 0; i < maxVertexId; i++) {
        int e = origGraphSrc[i + 1], s = origGraphSrc[i];
        for (int j = s; j < e; j++) {
            W_t(i, origGraphDst[j]) = 1.0;
        }
    }
    W_t = W_t.t();
    double sumRow = 0;
    double Degr = 0;
    vector<double> d;
    d.resize(maxVertexId);
    for (int i = 0; i < maxVertexId; i++) {
        sumRow = 0;
        for (int j = 0; j < maxVertexId; j++)
            sumRow += W_t(i, j);
        if (sumRow < 0.000001) {
            printf("node %d has no ingoing edges\n", i);
            d[i] = 1;
        } else {
            for (int j = 0; j < maxVertexId; j++)
                W_t(i, j) = W_t(i, j) / sumRow;
            d[i] = (sumRow - 1) / (1.0 * sumRow);
        }
        Degr += sumRow;
    }

    mat W = W_t.t();
    //W.print();
    for (int i = 0; i < maxVertexId; i++) {
        index[i] = i;
        revIndex[i] = i;
    }
    //reOrderW(W);   		
    cx_mat p;
    cx_vec eigval;
    cout << "eig(W) start" << endl;
    eig_gen(eigval, p, W);
    cout << "eig(W) end" << endl;
    for (int i = 0; i < maxVertexId; i++) {
        if (norm(eigval(i)) <= pow(10, -10))
            cout << "eigval(" << i << "): " << eigval(i) << endl;
    }
    p = normalise(p, 2, 0);
    cx_mat D = diagmat(eigval);
    //reOrderPD(p,D);


    /*
	for(int i=0;i<maxVertexId;i++)
		for(int j=0;j<maxVertexId;j++)
			p(i,j)=p(i,j)*cx_double(j+1,0);
    */

    cx_mat p_t_p = p.t() * p;
    cx_mat X = cx_mat(maxVertexId, maxVertexId);
    for (int i = 0; i < maxVertexId; i++)
        for (int j = 0; j < maxVertexId; j++) {
            X(i, j) = cx_double(1 - decayFactor, 0.0) * p_t_p(i, j) / D(i, i) /
                      (cx_double(1.0, 0.0) / D(i, i) - cx_double(decayFactor, 0) * D(j, j));
        }

    cx_mat U;
    cx_mat V;
    vec sigma;
    cx_mat inv_p = inv(p);
    svd(U, sigma, V, inv_p);
    L = inv_p.t() * X * U.cols(0, Rank - 1);
    R = diagmat(sigma.subvec(0, Rank - 1)) * (V.cols(0, Rank - 1)).t();

    l_ = mean(L, 1);
    l_dev = stddev(L, 1, 1);
    r_ = mean(R, 0);
    r_dev = stddev(R, 1, 0);

    //save variables
    L.save(Lpath);
    R.save(Rpath);
    l_.save(l_path);
    r_.save(r_path);
    l_dev.save(l_devpath);
    r_dev.save(r_devpath);
    FILE *fp = fopen(indexpath, "wb");
    fwrite(&index[0], sizeof(int), maxVertexId, fp);
    fclose(fp);
    FILE *fpRev = fopen(revIndexpath, "wb");
    fwrite(&revIndex[0], sizeof(int), maxVertexId, fpRev);
    fclose(fpRev);

}

void OptSimMatSVD::quickSortW(double *a, int s, int e, Mat<double> &M) {
    if (s > e)
        return;
    double x = a[s];
    int l = s, r = e;
    while (l < r) {
        while (l < r && a[r] <= x)
            r--;
        double temp = a[l];
        a[l] = a[r];
        a[r] = temp;
        M.swap_rows(r, l);
        M.swap_cols(r, l);

        int ttemp = revIndex[r];
        revIndex[r] = revIndex[l];
        revIndex[l] = ttemp;

        index[revIndex[r]] = r;
        index[revIndex[l]] = l;
        while (l < r && a[l] >= x)
            l++;
        temp = a[r];
        a[r] = a[l];
        a[l] = temp;
        M.swap_rows(r, l);
        M.swap_cols(r, l);

        ttemp = revIndex[r];
        revIndex[r] = revIndex[l];
        revIndex[l] = ttemp;

        index[revIndex[r]] = r;
        index[revIndex[l]] = l;

    }
    //a[r]=x;
    quickSortW(a, s, r - 1, M);
    quickSortW(a, r + 1, e, M);

}

void OptSimMatSVD::reOrderW(Mat<double> &M) {
    double a[maxVertexId];
    int count;
    for (int i = 0; i < maxVertexId; i++) {
        count = 0;
        while (count < maxVertexId && M(count, i) == 0) {
            count++;
        }
        if (count == maxVertexId)
            a[i] = 0;
        else
            a[i] = 1 / M(count, i);
    }
    //printArray(a,N-1);
    quickSortW(a, 0, maxVertexId - 1, M);
    /*
	cout<<"output a:"<<endl;
	printArray(a,N-1);
	M.print();
    */
}

void OptSimMatSVD::quickSortPD(double *a, int s, int e, Mat<cx_double> &p, Mat<cx_double> &D) {
    if (s > e)
        return;
    double x = a[s];
    int l = s, r = e;
    while (l < r) {
        while (l < r && a[r] >= x)
            r--;
        double temp = a[l];
        a[l] = a[r];
        a[r] = temp;
        p.swap_cols(r, l);
        cx_double dt = D(r, r);
        D(r, r) = D(l, l);
        D(l, l) = dt;
        while (l < r && a[l] <= x)
            l++;
        temp = a[r];
        a[r] = a[l];
        a[l] = temp;
        p.swap_cols(r, l);
        dt = D(r, r);
        D(r, r) = D(l, l);
        D(l, l) = dt;
    }
    //a[r]=x;
    quickSortPD(a, s, r - 1, p, D);
    quickSortPD(a, r + 1, e, p, D);
}

void OptSimMatSVD::reOrderPD(Mat<cx_double> &p, Mat<cx_double> &D) {
    double w[maxVertexId];
    memset(w, 0, sizeof(w));
    for (int i = 0; i < maxVertexId; i++) {
        for (int j = 0; j < maxVertexId; j++) {
            w[i] += std::norm(p(j, i)) / (j + 1);
        }
        w[i] = sqrt(w[i]);
    }
    quickSortPD(w, 0, maxVertexId - 1, p, D);
}

int OptSimMatSVD::getRevIndexX(int i) {
    return revIndex[i];
}

int OptSimMatSVD::getIndexX(int i) {
    return index[i];
}

