/*************************************************************************
    > File Name: EffiEVD.cpp
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Sun 22 Mar 2015 02:37:07 PM CST
 ************************************************************************/
#include "SimMatEVD.h"

void SimMatEVD::run(int qv, int k) {
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
        }
    }
//normal topk alg

    double score = 0.0;
    vector<SimRankValue> res;
    res.resize(maxVertexId);
    for (int i = 0; i < maxVertexId; i++) {
        score = getScore(qv, i);
        res[i].setVid(i);
        res[i].setValue(score);
    }
    save(res, k);

//method in paper:
/*	
	vector<SimRankValue> res;
	res.resize(k);	
	double theta=0;
//	priority_queue<pair<double,int>,vector<pair<double,int> >,greater<pair<double,int> > >coll;
	for(int i=0;i<k;i++){
	coll.push(make_pair(0,i));
	}
	cout<<"we have added k dummy nodes here"<<endl;
	
	double Su_qi,S_qi;
	for(int i=0;i<maxVertexId;i++){
		if(i==qv) continue;
		Su_qi=getSu(qv,i);
		if(Su_qi>=theta)
		{
			S_qi=getScore(qv,i);
		//	cout<<"we got S_qi"<<S_qi<<endl;
			if(S_qi>theta)
			{
				coll.pop();
				coll.push(make_pair(S_qi,i));
				theta = coll.top().first;
			}
		}
	}

	int count=0;
	while(!coll.empty()){
		cout<<"node:"<<coll.top().first<<"	score:"<<coll.top().second<<endl;
		res[count].setVid(coll.top().second);
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

void SimMatEVD::initialize() {
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
    vector<int> d;
    d.resize(maxVertexId);
    for (int i = 0; i < maxVertexId; i++) {
        sumRow = 0;//add an edge to itself
        for (int j = 0; j < maxVertexId; j++)
            sumRow += W_t(i, j);
        //W_t(i, i) = 1.0;
        if (sumRow < 0.0000001) {
            printf("node %d has no ingoing edges\n", i);
            d[i] = 1;
        } else {
            for (int j = 0; j < maxVertexId; j++)
                W_t(i, j) = W_t(i, j) / sumRow;
            d[i] = (sumRow - 1) / (1.0 * sumRow);
        }
        Degr += sumRow;
    }
    printf("total degree is %lf\n", Degr);

    mat W = W_t.t();
    cx_mat p;
    cx_vec eigval;
    eig_gen(eigval, p, W);
    cout << "eig_gen for W finished" << endl;

    for (int i = 0; i < maxVertexId; i++) {
        if (norm(eigval(i)) <= pow(10, -10))
            cout << "eigval(" << i << "): " << eigval(i) << endl;
    }
    cx_mat inv_p = inv(p);
    cout << "we are in EffiEVD initializing!" << Rank << endl;
    cx_mat p_t_p = p.t() * p;
    cx_mat X = cx_mat(maxVertexId, maxVertexId);
    cout << "start compute matrix X" << endl;
    for (int i = 0; i < maxVertexId; i++)
        for (int j = 0; j < maxVertexId; j++) {
            X(i, j) = cx_double(1 - decayFactor, 0.0) * p_t_p(i, j) /
                      (cx_double(1.0, 0.0) - cx_double(decayFactor, 0) * eigval(i) * eigval(j));
        }
    cout << "we got matrix X" << endl;
    cx_mat Q;
    vec lamda;
    eig_sym(lamda, Q, X);
    cx_mat Q_1 = inv(Q);
    Q = Q.submat(0, 0, maxVertexId - 1, Rank - 1);
    Q_1 = Q_1.submat(0, 0, Rank - 1, maxVertexId - 1);
    lamda = lamda.subvec(0, Rank - 1);
    //lamda.print();

    cout << "eig_gen(x) finished" << endl;
    L = inv_p.t() * Q;
    cout << "matrix L=inv(p.t)*Q got" << endl;
    R = diagmat(lamda) * Q_1 * inv_p;
    cout << "matrix R=lamda*inv(p*Q) got" << endl;


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

}
