//
// Created by yche on 1/16/18.
//

#ifndef LINSIM_BOOST_SERIALIZATION_EIGEN_H
#define LINSIM_BOOST_SERIALIZATION_EIGEN_H

#include <fstream>

#include <boost/serialization/array.hpp>
#include <boost/serialization/split_free.hpp>

#include <Eigen/Sparse>
#include <Eigen/Dense>

using namespace Eigen;

typedef Triplet<int> Trip;
using namespace std;

template<typename T, int whatever, typename IND>
void Serialize(ofstream &writeFile, SparseMatrix<T, whatever, IND> &m) {
    std::vector<Trip> res;
    int sz = m.nonZeros();
    m.makeCompressed();

    if (writeFile.is_open()) {
        IND rows, cols, nnzs, outS, innS;
        rows = m.rows();
        cols = m.cols();
        nnzs = m.nonZeros();
        outS = m.outerSize();
        innS = m.innerSize();

        writeFile.write((const char *) &(rows), sizeof(IND));
        writeFile.write((const char *) &(cols), sizeof(IND));
        writeFile.write((const char *) &(nnzs), sizeof(IND));
        writeFile.write((const char *) &(outS), sizeof(IND));
        writeFile.write((const char *) &(innS), sizeof(IND));

        writeFile.write((const char *) (m.valuePtr()), sizeof(T) * m.nonZeros());
        writeFile.write((const char *) (m.outerIndexPtr()), sizeof(IND) * m.outerSize());
        writeFile.write((const char *) (m.innerIndexPtr()), sizeof(IND) * m.nonZeros());

        writeFile.close();
    }
}

template<typename T, int whatever, typename IND>
void Deserialize(ifstream &readFile, SparseMatrix<T, whatever, IND> &m) {
    if (readFile.is_open()) {
        IND rows, cols, nnz, inSz, outSz;
        readFile.read((char *) &rows, sizeof(IND));
        readFile.read((char *) &cols, sizeof(IND));
        readFile.read((char *) &nnz, sizeof(IND));
        readFile.read((char *) &inSz, sizeof(IND));
        readFile.read((char *) &outSz, sizeof(IND));

        m.resize(rows, cols);
        m.makeCompressed();
        m.resizeNonZeros(nnz);

        readFile.read((char *) (m.valuePtr()), sizeof(T) * nnz);
        readFile.read((char *) (m.outerIndexPtr()), sizeof(IND) * outSz);
        readFile.read((char *) (m.innerIndexPtr()), sizeof(IND) * nnz);

        m.finalize();
        readFile.close();

    } // file is open
}

namespace boost {
    namespace serialization {

        template<class Archive, typename _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
        inline void save(Archive &ar,
                         const Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols> &M,
                         const unsigned int /* file_version */) {
            typename Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols>::Index rows = M.rows();
            typename Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols>::Index cols = M.cols();

            ar << rows;
            ar << cols;

            ar << make_array(M.data(), M.size());
        }

        template<class Archive, typename _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
        inline void load(Archive &ar,
                         Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols> &M,
                         const unsigned int /* file_version */) {
            typename Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols>::Index rows, cols;

            ar >> rows;
            ar >> cols;

            //if (rows=!_Rows) throw std::exception(/*"Unexpected number of rows"*/);
            //if (cols=!_Cols) throw std::exception(/*"Unexpected number of cols"*/);

            ar >> make_array(M.data(), M.size());
        }

        template<class Archive, typename _Scalar, int _Cols, int _Options, int _MaxRows, int _MaxCols>
        inline void load(Archive &ar,
                         Eigen::Matrix<_Scalar, Eigen::Dynamic, _Cols, _Options, _MaxRows, _MaxCols> &M,
                         const unsigned int /* file_version */) {
            typename Eigen::Matrix<_Scalar, Eigen::Dynamic, _Cols, _Options, _MaxRows, _MaxCols>::Index rows, cols;

            ar >> rows;
            ar >> cols;

            //if (cols=!_Cols) throw std::exception(/*"Unexpected number of cols"*/);

            M.resize(rows, Eigen::NoChange);

            ar >> make_array(M.data(), M.size());
        }

        template<class Archive, typename _Scalar, int _Rows, int _Options, int _MaxRows, int _MaxCols>
        inline void load(Archive &ar,
                         Eigen::Matrix<_Scalar, _Rows, Eigen::Dynamic, _Options, _MaxRows, _MaxCols> &M,
                         const unsigned int /* file_version */) {
            typename Eigen::Matrix<_Scalar, _Rows, Eigen::Dynamic, _Options, _MaxRows, _MaxCols>::Index rows, cols;

            ar >> rows;
            ar >> cols;

            //if (rows=!_Rows) throw std::exception(/*"Unexpected number of rows"*/);

            M.resize(Eigen::NoChange, cols);

            ar >> make_array(M.data(), M.size());
        }

        template<class Archive, typename _Scalar, int _Options, int _MaxRows, int _MaxCols>
        inline void load(Archive &ar,
                         Eigen::Matrix<_Scalar, Eigen::Dynamic, Eigen::Dynamic, _Options, _MaxRows, _MaxCols> &M,
                         const unsigned int /* file_version */) {
            typename Eigen::Matrix<_Scalar, Eigen::Dynamic, Eigen::Dynamic, _Options, _MaxRows, _MaxCols>::Index rows, cols;

            ar >> rows;
            ar >> cols;

            M.resize(rows, cols);

            ar >> make_array(M.data(), M.size());
        }

        template<class Archive, typename _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
        inline void serialize(Archive &ar,
                              Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols> &M,
                              const unsigned int file_version) {
            split_free(ar, M, file_version);
        }

        template<class Archive, typename _Scalar, int _Dim, int _Mode, int _Options>
        inline void serialize(Archive &ar,
                              Eigen::Transform<_Scalar, _Dim, _Mode, _Options> &t,
                              const unsigned int version) {
            serialize(ar, t.matrix(), version);
        }

        template<class Archive, typename _Scalar>
        void save(Archive &ar,
                  const Eigen::Triplet<_Scalar> &m,
                  const unsigned int /*version*/) {
            ar << m.row();
            ar << m.col();
            ar << m.value();
        }

        template<class Archive, typename _Scalar>
        void load(Archive &ar,
                  Eigen::Triplet<_Scalar> &m,
                  const unsigned int /*version*/) {
            typename Eigen::Triplet<_Scalar>::Index row, col;
            _Scalar value;

            ar >> row;
            ar >> col;
            ar >> value;

            m = Eigen::Triplet<_Scalar>(row, col, value);
        }

        template<class Archive, class _Scalar>
        void serialize(Archive &ar,
                       Eigen::Triplet<_Scalar> &m,
                       const unsigned int version) {
            split_free(ar, m, version);
        }

        template<class Archive, typename _Scalar, int _Options, typename _Index>
        void save(Archive &ar,
                  const Eigen::SparseMatrix<_Scalar, _Options, _Index> &m,
                  const unsigned int /*version*/) {
            _Index innerSize = m.innerSize();
            _Index outerSize = m.outerSize();

            typedef typename Eigen::Triplet<_Scalar> Triplet;
            std::vector<Triplet> triplets;

            for (_Index i = 0; i < outerSize; ++i)
                for (typename Eigen::SparseMatrix<_Scalar, _Options, _Index>::InnerIterator it(m, i); it; ++it)
                    triplets.push_back(Triplet(it.row(), it.col(), it.value()));

            ar << innerSize;
            ar << outerSize;
            ar << triplets;
        }

        template<class Archive, typename _Scalar, int _Options, typename _Index>
        void load(Archive &ar,
                  Eigen::SparseMatrix<_Scalar, _Options, _Index> &m,
                  const unsigned int /*version*/) {
            _Index innerSize;
            _Index outerSize;

            ar >> innerSize;
            ar >> outerSize;

            _Index rows = (m.IsRowMajor) ? outerSize : innerSize;
            _Index cols = (m.IsRowMajor) ? innerSize : outerSize;

            m.resize(rows, cols);

            typedef typename Eigen::Triplet<_Scalar> Triplet;
            std::vector<Triplet> triplets;

            ar >> triplets;

            m.setFromTriplets(triplets.begin(), triplets.end());
        }

        template<class Archive, typename _Scalar, int _Options, typename _Index>
        void serialize(Archive &ar, Eigen::SparseMatrix<_Scalar, _Options, _Index> &m, const unsigned int version) {
            split_free(ar, m, version);
        }

    } // namespace serialization
} // namespace boost

#endif //LINSIM_BOOST_SERIALIZATION_EIGEN_H
