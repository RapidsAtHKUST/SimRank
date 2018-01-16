//
// Created by yche on 1/16/18.
//

#include <fstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include "boost_serialization_eigen.h"
#include "../util/graph.h"

int main(int argc, char *argv[]) {
    if (file_exists("test.bin")) {
        Eigen::VectorXd D;
        std::ifstream ifs("test.bin");
        boost::archive::binary_iarchive ia(ifs);
        boost::serialization::load(ia, D, 0);
        ifs.close();
        for (auto i = 0; i < D.size(); i++) {
            cout << D[i] << endl;
        }
    } else {
        Eigen::VectorXd D;
        D.resize(10);
        for (auto i = 0; i < 10; i++) {
            D[i] = i * i + 0.1;
        }
        std::ofstream ofs("test.bin");
        boost::archive::binary_oarchive oa(ofs);
        boost::serialization::save(oa, D, 0);
        ofs.close();
        cout << "save finished" << endl;
    }
}