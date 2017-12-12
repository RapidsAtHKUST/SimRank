#include "tkde17_sim.h"
string TKDE17_RESULT_DIR = "./datasets/tkde17/";

LinearSystemSim::LinearSystemSim(string name, const DirectedG& g, float c_, float epsilon_){
    g_name = name;
    n = num_vertices(g);
    c = c_;
    epsilon = epsilon_;
    sim.resize(n,n);
    cout << get_file_path() << endl;
    if(file_exists(get_file_path()+".bin")){ // already computed, no need to run
        cout << "File already exists...." << endl;
        load();
    }else{
        sim.setZero();
        for(size_t i = 0; i < n;i++){
            sim(i,i) = 1;
        }
        run(g);
        cout << "saving to file: "<< get_file_path()<<endl;
        save();
    }
    cout << "simrank matrix size: " << sim.size() << endl;
}

void LinearSystemSim::Ax(SparseMatrix<float> &P, MatrixXf &x, MatrixXf &result){
    // compute Ax and save the result to result
    MatrixXf tmp(x);
    tmp = P.transpose() * tmp ;
    tmp = tmp * P;
    for(size_t i = 0;i < n;i++){ // set the diagonol to 0
        tmp(i,i) = 0;
    }
    result.noalias() = x - c * tmp;
}

void LinearSystemSim::run(const DirectedG & g){
    // build the transition matrix
    auto start = std::chrono::high_resolution_clock::now();
    SparseMatrix<float> P(n,n);
    indegree_mat(g, P);

    // solve the linear system
    auto & x = sim;
    float rho = 1, alpha = 1, w = 1;
    MatrixXf r(n,n), r0(n,n), v(n,n), p(n,n), s(n,n), t(n,n);
    r.setZero(); // current residual
    r0.setZero();
    v.setZero();
    p.setZero();
    s.setZero();
    t.setZero();
    for(size_t i = 0;i<n;i++){ // init r0
        r(i,i) = 1;
        r0(i,i) = 1;
    }
    size_t i = 0;
    float r_max = (1-c) * epsilon;
    auto eps2 = NumTraits<float>::epsilon()*NumTraits<float>::epsilon();
    float r0_sqnorm = r0.squaredNorm();
    while(i < maxIters  && r.cwiseAbs().maxCoeff() > r_max){// the stop criteria
        // cout << "i: " << i << endl;
        // cout << "max elemens in r:" <<r.cwiseAbs().maxCoeff() <<endl;
        // cout << r << endl;
        float rho_old = rho;
        rho = r0.cwiseProduct(r).sum();

        // restart when r0 is too orthogonol to r
        if(abs(rho) < eps2 * r0_sqnorm){
            // cout << "orthogonol with r!!!" << endl;
            Ax(P,x,r);
            r = -r;
            for(size_t i = 0;i<n;i++){
                r(i,i) += 1;
            }
            r0 = r;
            rho = r0_sqnorm = r.squaredNorm();
        }

        float beta = (rho / rho_old) * (alpha / w);
        p = r + beta * (p - w * v);
        Ax(P,p,v);

        alpha = rho / (r0.cwiseProduct(v).sum());
        s = r - alpha * v;
        Ax(P,s,t);

        auto tmp = t.squaredNorm();
        if(tmp > 0){
            w = t.cwiseProduct(s).sum() / tmp;
        }else{
            w = 0;
        }
        x += alpha * p + w * s;
        r = s - w * t;
        i ++;
    }
    // cout << x << endl;
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cpu_time = elapsed.count();
    mem_size = getValue(); 
}
void LinearSystemSim::save(){
    // save to disk
    write_binary((get_file_path()+".bin").c_str(), sim);
    // save the meta data
    ofstream out(get_file_path()+".meta");
    out << cpu_time << endl;
    out << mem_size << endl;
    out.close();
}     
void LinearSystemSim::load(){
    // load from disk
    read_binary((get_file_path()+".bin").c_str(), sim);
}



int tkde17_sim()
{
	int n = 5;
	int m = 10;
	SparseMatrix<float> A(n, n); // sparse matrix, column wise
	VectorXi row(2 * m); // source node array
	row.setZero();
	VectorXi col(2 * m); // target node array
	col.setZero();
	int count = 0;
	try
	{
		std::string line;
		ifstream file;
		ofstream fout;
		string input;

		file.open("simple.txt");             //////////Graph!!!!!!!!!

		while (!file.eof())
		{
			int a, b;
			getline(file, line);
			a = atoi(line.c_str());
			getline(file, line);
			b = atoi(line.c_str());
            cout << a << " " << b << endl;
			row(count) = a;
			col(count) = b;
			count++;
		}
		file.clear();
		file.close();
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	std::vector<Trip> trp; // triple (source, target, weight)

	for (int i = 0; i < count; i++)
	{
		trp.push_back(Trip(row(i), col(i), 1.0));
	}
	A.setFromTriplets(trp.begin(), trp.end()); // fill up the adjacency matrix
	
	float c = 0.8;                       /////Parameter c!!!!!!!!!!!!!!!
	int mn = A.nonZeros(); // the number of edges
	VectorXf out(n + 1); // positions of starting column in the value/inner array
	for (int i = 0; i < n; i++)
		out(i) = *(A.outerIndexPtr() + i);
	out(n) = mn;
	VectorXf in(mn);
	VectorXf val(mn);
	for (int i = 0; i< mn; i++)
	{
		in(i) = *(A.innerIndexPtr() + i);
		val(i) = *(A.valuePtr() + i);
	}
	MatrixXf rownumber(n, 1); // a column vector for in-degrees
	for (int i = 0; i < n; i++)
	{
		rownumber(i, 0) = A.col(i).sum();
	}
	MatrixXf indegreematrix(n, n); // in-degrees for node-pair graph
	indegreematrix = rownumber * rownumber.transpose();
	MatrixXf r(n, n);
	r = c*A;
	r = r*A;
	r.diagonal() = indegreematrix.diagonal();
	indegreematrix = indegreematrix.cwiseInverse(); // inverse each element
	float alpha = 0;
	float beta = 0;
	float zr = 0;
	float ptp = 0;
	float tz = 0;
	MatrixXf x(n, n);              ///SimRank !!!!!!!!!!!!!
	x.setZero();
	MatrixXf p(n, n);
	p = r;
	p = p.cwiseProduct(indegreematrix);
	MatrixXf temp(n, n);
	A.resize(0, 0);
	MatrixXf temp2(n, n);
	temp2.setZero();
	VectorXf v(n);
	v.setZero();

	for (int k = 0; k < 100; k++)
	{
		temp = p;
		temp.diagonal().setZero();
		temp2.setZero();
		count = 0;
		for (int i = 0; i < n; i++)
		{
			v.setZero();
			for (int j = out(i); j <= out(i + 1) - 1; j++)
			{
				v = v + val(j) * temp.col(in(j));
			}
			count = 0;
			for (int mi = 0; mi < n; mi++)
			{
				for (int ni = out(mi); ni <= out(mi + 1) - 1; ni++)
				{
					temp2(mi, i) += val(ni) * v(in(ni));
				}
			}
		}
		temp = temp2;
		temp.diagonal().setZero();
		temp = p.cwiseProduct(indegreematrix.cwiseInverse()) - c*temp;
		zr = (r.cwiseProduct(r)).cwiseProduct(indegreematrix).sum();
		ptp = p.cwiseProduct(temp).sum();
		alpha = zr / ptp;
		x = x + alpha * p;
		if (alpha*p.cwiseAbs().maxCoeff() < 0.0001)      // Error Bound!!!!!!!!!
		{
			break;
		}
		r = r - alpha * temp;
		tz = r.cwiseProduct(temp).cwiseProduct(indegreematrix).sum();
		beta = tz / ptp;
		p = r.cwiseProduct(indegreematrix) + beta * p;
	}
	cout << x << endl;
}

void tkde17_sim(DirectedG & g, float c, float epsilon){
    int n = num_vertices(g);
    int m = num_edges(g);
    cout << c << " " << epsilon << endl;
    cout << n  << " " << m << endl;

    SparseMatrix<float> A(n, n);
    VectorXi row(2 * m);
    row.setZero();
    VectorXi col(2 * m);
    col.setZero();
    int count = 0;
    DirectedG::edge_iterator edge_iter, edge_end;
    tie(edge_iter, edge_end) = edges(g);
    for(;edge_iter!=edge_end;edge_iter++){
        int a, b;
        a = source(*edge_iter, g);
        b = target(*edge_iter, g);
        cout << a << " " << b << endl;
        row(count) = a;
        col(count) = b;
        count++;
    }
    // try
    // {
    // 	std::string line;
    // 	ifstream file;
    // 	ofstream fout;
    // 	string input;

    // 	file.open("simple.txt");             //////////Graph!!!!!!!!!

    // 	while (!file.eof())
    // 	{
    // 		int a, b;
    // 		getline(file, line);
    // 		a = atoi(line.c_str());
    // 		getline(file, line);
    // 		b = atoi(line.c_str());
    // 		row(count) = a;
    // 		col(count) = b;
    // 		count++;
    // 	}
    // 	file.clear();
    // 	file.close();
    // }
    // catch (std::exception& e)
    // {
    // 	std::cout << e.what() << std::endl;
    // }

    std::vector<Trip> trp;

    for (int i = 0; i < count; i++)
    {
        trp.push_back(Trip(row(i), col(i), 1.0));
    }
    A.setFromTriplets(trp.begin(), trp.end());

    // float c = 0.8;                       /////Parameter c!!!!!!!!!!!!!!!
    int mn = A.nonZeros();
    VectorXf out(n + 1);
    for (int i = 0; i < n; i++)
        out(i) = *(A.outerIndexPtr() + i);
    out(n) = mn;
    VectorXf in(mn);
    VectorXf val(mn);
    for (int i = 0; i< mn; i++)
    {
        in(i) = *(A.innerIndexPtr() + i);
        val(i) = *(A.valuePtr() + i);
    }
    MatrixXf rownumber(n, 1);
    for (int i = 0; i < n; i++)
    {
        rownumber(i, 0) = A.col(i).sum();
    }
    MatrixXf indegreematrix(n, n);
    indegreematrix = rownumber * rownumber.transpose();
    MatrixXf r(n, n);
    r = c*A;
    r = r*A;
    r.diagonal() = indegreematrix.diagonal();
    indegreematrix = indegreematrix.cwiseInverse();
    float alpha = 0;
    float beta = 0;
    float zr = 0;
    float ptp = 0;
    float tz = 0;
    MatrixXf x(n, n);              ///SimRank !!!!!!!!!!!!!
    x.setZero();
    MatrixXf p(n, n);
    p = r;
    p = p.cwiseProduct(indegreematrix);
    MatrixXf temp(n, n);
    A.resize(0, 0);
    MatrixXf temp2(n, n);
    temp2.setZero();
    VectorXf v(n);
    v.setZero();

    for (int k = 0; k < 100; k++)
    {
        temp = p;
        temp.diagonal().setZero();
        temp2.setZero();
        count = 0;
        for (int i = 0; i < n; i++)
        {
            v.setZero();
            for (int j = out(i); j <= out(i + 1) - 1; j++)
            {
                v = v + val(j) * temp.col(in(j));
            }
            count = 0;
            for (int mi = 0; mi < n; mi++)
            {
                for (int ni = out(mi); ni <= out(mi + 1) - 1; ni++)
                {
                    temp2(mi, i) += val(ni) * v(in(ni));
                }
            }
        }
        temp = temp2;
        temp.diagonal().setZero();
        temp = p.cwiseProduct(indegreematrix.cwiseInverse()) - c*temp;
        zr = (r.cwiseProduct(r)).cwiseProduct(indegreematrix).sum();
        ptp = p.cwiseProduct(temp).sum();
        alpha = zr / ptp;
        x = x + alpha * p;
        if (alpha*p.cwiseAbs().maxCoeff() < epsilon)      // Error Bound!!!!!!!!!
        {
            break;
        }
        r = r - alpha * temp;
        tz = r.cwiseProduct(temp).cwiseProduct(indegreematrix).sum();
        beta = tz / ptp;
        p = r.cwiseProduct(indegreematrix) + beta * p;
    }
    cout << x << endl;
}
