# the linearization method of "Efficient SimRank Computation via Linearization"
import random
from collections import Counter

from sklearn import preprocessing

from simrank import *


# from memory_profiler import profile

def SLD_and_SLD_estimate(g, D, R, k, T, c=0.6):
    '''
    g: the graph
    D: current diagonal matrix, a length n array
    R: number of samples to estimate the S^{L}(D)_{kk} and S^(E)(D)_{kk}
    k: k
    T: length of random walk
    '''
    n = g.number_of_nodes()
    position = np.zeros(R, dtype="int")  # the hashtable for record position of each walker at each step
    position[:] = k
    distribution = np.zeros(n, dtype="float")
    alpha = 0  # initialize the estimated S^{L}(D)_{kk}
    beta = 0  # initialize the estimated S^{L}(Ek)_{kk}
    for t in np.arange(0, T):  # t: the iteration
        # clear the distribution vector
        distribution[:] = 0
        # rebuild the distribution
        for p in position:
            if p >= 0:  # only consider active node, ignore dead nodes
                distribution[p] += 1 / R
        # update alpha
        delta_alpha = np.dot(distribution.T, np.multiply(D, distribution))
        alpha += (c ** t) * delta_alpha
        # update beta
        beta += (c ** t) * (distribution[k] ** 2)
        # move to next step
        for p in np.nditer(position, op_flags=['readwrite']):
            if p >= 0:  # only consider active node
                in_edges = g.in_edges(int(p))
                if len(in_edges) > 0:
                    p[...] = random.choice(in_edges)[0]
                else:  # has no in-neighrbo
                    p[...] = -1  # move to dead node
    return (alpha, beta)


def compute_D(g, L=3, R=100, T=11, c=0.6):
    '''
    compute the diagonal estimation matrix
    g: the graph
    L: #iterations
    R: number of samples
    T: length of random walk
    '''
    n = g.number_of_nodes()
    D = np.zeros(n, dtype="float")
    D[:] = 1  # initial of D
    for iteration in range(0, L):
        for k in np.arange(0, n):
            if k % 100 == 0:
                print("iteration and k", iteration, k)
            alpha, beta = SLD_and_SLD_estimate(g, D, R, k, T, c)
            delta = (1 - alpha) / beta
            D[k] += delta
    return D


# @profile
def linear_single_pair(g, pair, D=None, R=100, T=11, c=0.6):
    '''
    g: the directed graph
    D: the diagonal correction matrix, is a numpy array
    pair: the query pair, tuple
    R: number of samples
    T: length of random walk
    '''
    i, j = pair
    n = g.number_of_nodes()
    if i == j:
        return 1
    p_i = np.zeros(R, dtype="int")  # position of walker from i
    p_j = np.zeros(R, dtype="int")  # position of walker from j
    p_i[:] = i
    p_j[:] = j
    sim_score = 0
    if D is None:
        D = compute_D(g, c=c)
    for t in range(0, T):
        pi_dict = Counter(p_i)
        pj_dict = Counter(p_j)
        intersect = np.intersect1d(p_i, p_j)
        for p in intersect:
            if p >= 0:  # only count the valid position
                sim_score += (c ** t) * D[p] * (pi_dict[int(p)]) * (pj_dict[int(p)]) / (R ** 2)
        # move to next step
        for p in np.nditer(p_i, op_flags=['readwrite']):
            if p >= 0:
                in_edges_pi = g.in_edges(int(p))
                if len(in_edges_pi) > 0:
                    p[...] = random.choice(in_edges_pi)[0]
                else:  # has no in-neighbors
                    p[...] = -1
        for p in np.nditer(p_j, op_flags=['readwrite']):
            if p >= 0:
                in_edges_pj = g.in_edges(int(p))
                if len(in_edges_pj) > 0:
                    p[...] = random.choice(g.in_edges(int(p)))[0]
                else:  # has no in-neighbors
                    p[...] = -1
    return sim_score


def iter_D_single_pair(AT, pair, D=None, P=None, T=11, c=0.6):
    '''
    AT: A.T, csr format
    pair: query pair
    D: the diagonal
    P: column normalized matrix
    '''
    i, j = pair
    if i == j:
        return 1
    n = AT.shape[0]
    if D is None:
        D = compute_D(g, c)
    if P is None:
        P_T = preprocessing.normalize(AT, norm='l1', axis=1)
        P = P_T.T
    # print("PT: ")
    # print(P.toarray())
    sim_score = 0
    # make e_i
    e_i = np.zeros(n, dtype="float")
    e_i[i] = 1
    # make e_j
    e_j = np.zeros(n, dtype="float")
    e_j[j] = 1
    x = e_i
    y = e_j
    for t in range(0, T):
        # update simrank
        tmp = np.multiply(x, D)
        delta = np.dot(tmp, y)
        sim_score += (c ** t) * delta
        # update x and y
        x = P.dot(x)
        y = P.dot(y)
    return sim_score


def iter_F_sinel_pair(AT, pair, Ft=None, c=0.6, T=11, z=None):
    '''
    AT: A.T
    pair: query pair
    F: the F
    '''
    i, j = pair
    if i == j:
        return 1
    n = AT.shape[0]
    AT = AT.tocsr()
    index = j * n + i
    if Ft is None:
        F = make_associate_F(AT, AT)
        Ft = F.transpose()
    e_i = np.zeros(n ** 2, dtype="float")
    e_i[index] = 1
    if z == None:
        z = make_z(n)
    q = e_i
    p = np.zeros(n ** 2, dtype="float")
    for t in range(0, T):
        q = Ft.dot(q)
        p += q
    sim_score = np.inner(z, p)
    return sim_score


def make_z(n):
    index_of_zero_rows_of_F = make_index_of_vec_n(n)
    z = np.zeros(n ** 2, dtype="float")
    z[index_of_zero_rows_of_F] = 1
    return z


def make_associate_F(AT, c=0.6):
    AT = AT.tocsr()
    n = AT.shape[0]
    F = csr_kron_F(AT, AT)
    index_of_zero_rows_of_F = make_index_of_vec_n(n)
    csr_rows_set_nz_to_val(F, index_of_zero_rows_of_F, 0)
    F.setdiag(0)
    F *= c
    return F


def test():
    for M in [A, B, C, E]:
        m = adj_mat(M)
        g = nx.from_numpy_matrix(m.toarray(), create_using=nx.DiGraph())
        print(m.toarray())
        print(g.number_of_nodes())
        d = compute_D(g)
        mT = m.transpose().tocsr()
        asocia_F = make_associate_F(mT).transpose().tocsr()
        truth = simrank(m.transpose())
        print(truth)
        print("m.shape", m.shape)
        pairs = [(i, j) for i in range(0, m.shape[0]) for j in range(i, m.shape[0])]
        for pair in pairs:
            s = linear_single_pair(g, pair, d)
            truncated = truncated_MC(g, pair, R=100)
            c_stop = c_MC(g, pair, R=100)
            iter_r = iter_D_single_pair(mT, pair, D=d)
            iter_F = iter_F_sinel_pair(mT, pair, Ft=asocia_F)
            print(pair, truth[pair], s, truncated, c_stop, iter_r, iter_F)
            # print(pair, truth[pair], iter_single_pair(m, pair))
    return


if __name__ == '__main__':
    test()
