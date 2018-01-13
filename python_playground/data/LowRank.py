# the low-rank method for simrank
# Fast computation of SimRank for static and dynamic information networks (EDBT'10)
from memory_profiler import profile

from simrank import *
from utils import make_index_of_vec_n

np.set_printoptions(precision=2)


def preprocess(AT, k=5, IE=False, c=0.6):
    '''
    AT: A.T, csr sparse matrix
    k: number of singular values
    IE: whether has the (I-E) term, True would be correct
    return: (K_u, Sigma_inverse, K_v, V_r)
    '''
    if IE == False:
        print("computing False lowrank")
    else:
        print("computing True lowrank")
    n = AT.shape[0]
    # print("AT:")
    # print(AT.toarray())
    # normalized AT
    print("normalizing ...")
    PT = preprocessing.normalize(AT, norm='l1', axis=1)
    PT = PT.astype("float32")  # reduce the precision to use more memory
    print("computing SVD...", "k: ", k)
    (U, S, VT) = sparse.linalg.svds(PT, k, which="LM")
    print("computing kronecker product...")
    K_u = np.kron(U, U)
    K_sigma = np.kron(S, S)
    K_sigma_inverse = 1 / K_sigma
    K_v = np.kron(VT, VT)
    print(K_u)
    print(K_v)
    print("K_v size", K_v.nbytes)
    index_of_zero_rows = make_index_of_vec_n(n)
    if IE == True:  # need to multiply I-E to K_u
        K_u[index_of_zero_rows] = 0  # set rows to zeros
    # compute Sigma
    print("computing sigma..")
    Sigma = np.diag(K_sigma_inverse) - c * np.dot(K_v, K_u)
    Sigma_inverse = np.linalg.inv(Sigma)
    # build vec(I)
    vec_I = np.zeros(n ** 2, dtype="int")
    vec_I[index_of_zero_rows] = 1
    print("computing V_r")
    V_r = np.dot(K_v, vec_I)
    print("finish indexing")
    return (K_u, Sigma_inverse, K_v, V_r)


@profile
def lowrank_simrank(AT, indices=None, IE=False, k=5, c=0.6):
    '''
    Direct method for simrank
    '''
    print("low rank for simrank")
    n = AT.shape[0]
    I = np.eye(n)
    vec_I = I.reshape(n ** 2)
    if indices == None:
        indices = preprocess(AT, k, IE, c)  # get the offline indices
    (K_u, Sigma_inverse, K_v, V_r) = indices
    print("finish indexing, now compute low rank approximation... ")
    if IE == False:  # the incorrect way
        vec_S = (1 - c) * (vec_I + c * np.dot(K_u, np.dot(Sigma_inverse, V_r)))
    else:
        vec_S = (vec_I + c * np.dot(K_u, np.dot(Sigma_inverse, V_r)))
    print("reshaping")
    S = vec_S.reshape((n, n))
    return S


def test():
    # m = adj_mat(A)
    # print(type(m))
    # print(type(m.transpose()))
    # S = simrank(m.transpose())
    # print("true simrank")
    # print(S)
    # ls = lowrank_simrank(m.transpose(), k=3, IE=True)
    # print(np.around(ls, 2))
    g = load_sparse_csr("./datasets/adj_T/ca-HepTh.npz")
    lowrank_simrank(g, k=10, IE=True)


if __name__ == '__main__':
    test()
