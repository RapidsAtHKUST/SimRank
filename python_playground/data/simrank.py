import itertools
import time

import networkx as nx
from scipy.sparse.linalg import inv
from sklearn import preprocessing

from utils import *

# set the printing config
np.set_printoptions(precision=4)

ITERATIONS = 50

# test graphs
A = [
    (3, 1),
    (3, 2),
    (3, 4),
    (3, 5),
    (4, 6),
    (5, 6),
    (6, 7),
    (6, 8)
]
B = [
    (1, 2),
    (1, 3),
    (1, 4),
    (2, 1),
    (3, 1),
    (4, 1)
]
C = [
    (1, 2),
    (1, 3),
    (2, 1),
    (2, 3),
    (3, 1),
    (3, 2)
]
D = [
    (1, 2),
    (1, 3),
    (2, 1),
    (3, 1)
]

E = [
    (1, 4),
    (2, 1),
    (3, 1),
    (3, 2),
    (4, 3)
]

H = [
    (1, 2),
    (1, 3),
    (1, 4)
]


class gmres_counter(object):
    def __init__(self, disp=True):
        self._disp = disp
        self.niter = 0
        self.ticker = time.time()
        self.results = []

    def __call__(self, rk=None):
        self.niter += 1
        now = time.time()
        delta = now - self.ticker
        self.results.append((self.niter, rk, delta))
        if self._disp:
            print(self.niter, str(rk), delta)
        self.ticker = now


def adj_mat(edge_list):
    '''
    given an edge list, return the adjacency matrix
    '''
    g1 = nx.DiGraph()
    g = nx.from_edgelist(edge_list, create_using=g1)
    g = nx.convert_node_labels_to_integers(g)
    return nx.to_scipy_sparse_matrix(g)


def sort_sim(A):
    '''
    given a simrank matrix, sort simran scores
    output is a sorted list(node pair, value)
    '''
    # convert a matrix to (pos, value) tuples
    n = A.shape[0]
    sims = list()
    for i in range(0, n):
        for j in range(0, n):
            pos = (i + 1, j + 1)
            sims.append((pos, A[i][j]))
    sims = sorted(sims, key=lambda x: x[1], reverse=True)
    return sims


def false_sim(A, c=0.6):
    '''
    S = cPSP + (1-c)I
    '''

    n = A.shape[0]
    P = preprocessing.normalize(A, 'l1', axis=0)
    I_n = np.eye(n)
    I_nn = np.eye(n * n)
    a = (I_nn - c * np.kron(P.T, P.T))
    b = (1 - c) * I_n.reshape((I_n.size, 1))
    S = np.linalg.solve(a, b).reshape((n, n))
    print("False Simrank")
    # print(S)
    return S


def symmetric_simrank(A, c=0.6, F=None, return_info=False, format_shape=True, use_method='gmres'):
    '''
    corrent the symmetric linear system methods
    A: a csr matrix of A.T (already transposed)
    return_info: whether return the iteration information
    format_shape: whether return the matrix as the square type
    '''
    n = A.shape[0]
    A = A.tocsr()
    print(matrix_info(A))
    size = int(n * (n + 1) / 2)
    if F is None:
        F = parallel_symmetric_csr_kron(A)

    # make index of symVec(I_n)
    index_of_zero_rows_of_F = np.array([
        (i - 1) * n - int((i - 1) * (i - 2) / 2) for i in range(1, n + 1)
    ])
    # multiply by (I-E)
    print("multiplying I-E")
    csr_rows_set_nz_to_val(F, index_of_zero_rows_of_F, 0)
    print("finish computing F: ", matrix_info(F))
    print("generating b")  # b is svec(I)
    # svec_I = np.array([1 if i == j else 0 for i in range(0, n) for j in range(i, n)], dtype="float")
    length_of_b = int((n + 1) * n / 2)
    svec_I = np.zeros(size, dtype="int")
    svec_I[index_of_zero_rows_of_F] = 1
    b = svec_I
    # print("vector b", b.shape)
    # print(b)
    if use_method == 'gmres':
        print(matrix_info(F))
        print("multiplying by -c")
        F *= (-c)
        print("adding diagonal with 1")
        I_nn = sparse.eye(size, dtype="int")
        print(matrix_info(I_nn))
        F += I_nn
        # print("eliminating zeros")
        # F.eliminate_zeros()
        a = F
        print("finish computing a")
        # print(a.toarray())

        print("solving the linear system")
        if return_info == False:
            svec_S, code = sparse.linalg.gmres(a, b)
            print("finish computing symmetric simrank, svec_S")
        else:
            counter = gmres_counter()
            svec_S, code = sparse.linalg.gmres(a, b, \
                                               callback=counter, \
                                               maxiter=ITERATIONS, \
                                               tol=1e-50, \
                                               restart=50, \
                                               x0=b)
        if code >= 0:
            if format_shape:
                print(svec_S)
                print("reshaping to S...")
                S = np.zeros((n, n), dtype="float")  # initialize the simrank matrix
                for index in np.ndindex(n, n):
                    x = index[0]
                    y = index[1]
                    # query the simrank score
                    query_position = get_row_number(x, y, n)
                    if x == y:
                        S[x][y] = 1
                    else:
                        S[x][y] = math.sqrt(2) * \
                                  svec_S[query_position] / 2
            else:
                S = svec_S
            if return_info:
                return (S, counter.results)
            else:
                return S
        else:
            print("Error occured whild solving the linear system..")
            return -1
    elif use_method == 'jacobi':
        print("multiply by c")  # notice that it is positive
        F *= c
        print("setting diag to 0")
        F.setdiag(0)
        # initial x
        x = b
        results = []
        t = 0
        ticker = time.time()
        while t < ITERATIONS:
            tmp = x
            x = F.dot(x)
            x[index_of_zero_rows_of_F] = 1
            residual = np.linalg.norm(x - tmp)
            now = time.time()
            delta = now - ticker
            print("iteration: ", t, " residual: ", residual, delta)
            ticker = now
            results.append((t, residual, delta))
            t += 1
        svec_S = x
        S = svec_S
        if format_shape:
            # return (n,n) SimRank matrix
            print(svec_S)
            print("reshaping to S...")
            S = np.zeros((n, n), dtype="float")  # initialize the simrank matrix
            for index in np.ndindex(n, n):
                x = index[0]
                y = index[1]
                # query the simrank score
                query_position = get_row_number(x, y, n)
                if x == y:
                    S[x][y] = 1
                else:
                    S[x][y] = math.sqrt(2) * svec_S[query_position] / 2
        if return_info:
            return (S, results)
        else:
            return S
    else:
        pring("method doesn't support")
        return


def sym_simrank(A, c=0.6):
    '''
    new linear system using symmetric kronecker product
    '''
    n = A.shape[0]
    print("Symmetric kronecker SimRank", "n: ", n)
    print("number of edges: ", np.around(np.count_nonzero(A)))
    Q = getQ(n)
    size = int((n + 1) * n / 2)

    def svec(M):
        '''
        return svec of a symmetric matrix
        '''
        r = np.dot(Q, M.reshape((M.size, 1)))
        print(r.shape)
        return r

    P = preprocessing.normalize(A, 'l1', axis=0)
    print("P.T: ")
    print(P.T)
    P_kron_P = np.kron(P.T, P.T)
    sym_kron_P = np.dot(Q, np.dot(P_kron_P, Q.T))
    # print("Q")
    # print(np.around(Q, decimals=1))
    # print("kronecker P", "number of nonzeros", np.count_nonzero(P_kron_P))
    # print(P_kron_P)
    print("symmetric kronecker P", "number of nonzeros", np.count_nonzero(sym_kron_P))
    print(sym_kron_P)
    I_nn = np.eye(size)
    I_n = np.eye(n)
    E = np.diagflat(np.eye(n))
    E_s = np.dot(Q, np.dot(E, Q.T))
    F = I_nn - E_s
    print("I-E:")
    print(F)

    F_s = np.dot(F, sym_kron_P)
    a = I_nn - c * F_s
    print("a")
    print(a)
    # print("F_s: ")
    # print(np.dot(F, sym_kron_P))
    # print("Graph of F_s")
    # print(nonzeros_to_edges_for_nonsymmetric_situation(np.dot(F, sym_kron_P), n))
    # print("number of nonzeros of F_s", np.count_nonzero(c*np.dot(F, sym_kron_P)))
    b = svec(I_n)
    # print("b")
    # print(b)
    # print("latex code of F_s")
    # print(matrix_to_latex_code(np.dot(F, sym_kron_P)))
    svec_S = np.linalg.solve(a, b)
    vec_S = np.dot(Q.T, svec_S)
    S = vec_S.reshape((n, n))
    print("solution of symmtrix linear system")
    print(S)


def iter_simrank(A, c=0.6, epsilon=0.01):
    '''
    the original iterative method of simrank
    A:  a csr format sparse matrix
    '''
    print("KDD'02 naive iterative method of all pairs simrank")
    print("type of A", type(A))
    n = A.shape[0]
    print("information of A, ", type(A), "size is ", A.data.nbytes, " bytes.")
    P = preprocessing.normalize(A, 'l1', axis=0)  # normalize along each column
    print("type of P", type(P))
    t = math.ceil(math.log(epsilon, c))
    P_t = P.transpose()
    # pdb.set_trace()
    S = sparse.eye(n, dtype="float64").tocsr()
    # S = np.eye(n, dtype="float64")
    t1 = time.time()
    print("type of s  ", type(S))
    for i in range(0, t):
        print("iteration: ", i)
        S = P_t.dot(S)
        S = S.dot(P)
        S = c * S
        S.setdiag(1)
        # np.fill_diagonal(S, 1)
    t2 = time.time()
    print("finish in", t2 - t1, "seconds")
    return S


def adj_coo_F(A):
    '''
    compute the adj_F by using scipy kronecker product
    '''
    n = A.shape[0]
    print("computing kronecker product")
    F = sparse.kron(A, A)
    print("adj csr F: ", matrix_info(F))
    return F


def adj_F(A):
    '''
    A: the adjacency matrix, as CSR format
    build the node_pair graph G(F)
    return: the sparse F=(I-E)Pt Pt
    '''
    n = A.shape[0]
    GF = nx.DiGraph()
    print("adding nodes...")
    GF.add_nodes_from(range(0, n * n))  # pre-define all nodes ids
    print("finishe")
    GF_edges = list()

    def node_pair_id(x, y):
        '''
        return the node pair id of G(F)
        '''
        return x * n + y

    g = nx.from_scipy_sparse_matrix(A, create_using=nx.DiGraph())
    edge_tuples = itertools.product(g.edges_iter(), repeat=2)
    print("adding edges...")
    GF.add_edges_from(
        (node_pair_id(e[0][0], e[1][0]), node_pair_id(e[0][1], e[1][1])) for e in edge_tuples if e[0][1] != e[1][1])
    print("finish, now convert to csr format")
    F = nx.to_scipy_sparse_matrix(GF, dtype="bool", format='csr')
    print("finish..")
    return F


def jacobi_simrank(AT, F=None, iteration=ITERATIONS, c=0.6, return_info=False):
    '''
    iterative Jacobi method for simrank
    AT: A.T
    iteration: number of iterations
    F: the associated matrix
    c: the decay factor
    return_info: whether return the statistical results
    return: S, iteration and residual, time
    '''
    results = []
    A = AT
    print("Jacobi method for simrank")
    print("A", matrix_info(A))
    n = A.shape[0]
    A = A.tocsr()
    # compute F: (I-E) Pt \otimes Pt
    print("computing A * A ...")
    if F is None:
        F = csr_kron_F(A, A)
    print("Finished...")

    # # mimic (I-E) multiplication
    print("multiply by I-E")
    print("F.indprt length", len(F.indptr))
    print("F matrix", matrix_info(F))
    index_of_zero_rows_of_F = make_index_of_vec_n(n)
    csr_rows_set_nz_to_val(F, index_of_zero_rows_of_F, 0)
    print("Finished..")

    print("finish computing F...", matrix_info(F))

    print("multiply by c")  # notice that it is positive
    F *= c
    print("setting diag to 0")
    F.setdiag(0)

    # initial x
    I = np.eye(n, dtype="float")
    x = I.reshape(n ** 2)
    results = []

    t = 0
    ticker = time.time()
    while t < iteration:
        tmp = x
        x = F.dot(x)
        x[index_of_zero_rows_of_F] = 1
        residual = np.linalg.norm(x - tmp)
        now = time.time()
        delta = now - ticker
        print("iteration: ", t, " residual: ", residual, delta)
        ticker = now
        results.append((t, residual, delta))
        t += 1
    S = x.reshape((n, n))
    if not return_info:
        return S
    else:
        return (S, results)


def simrank(A, c=0.6, F=None, return_info=False, format_shape=True):
    """
    Correct SimRank Algorithm
    A: csr format , transeposed adjacency matrix, dtype = bool
    F: a csr row normalized adj matrix for A.T \otime A.T
    return_info: whether return the statistical results
    return: S, iteration and residule, time
    """
    # print("Correnting corrent Simrank")
    # print("A", matrix_info(A))

    n = A.shape[0]
    A = A.tocsr()

    # compute F: (I-E) Pt \otimes Pt
    # print("computing A * A ...")
    if F is None:
        F = csr_kron_F(A, A)
    # print("Finished...")

    # # mimic (I-E) multiplication
    # print("multiply by I-E")
    # print("F.indprt length",len(F.indptr))
    # print("F matrix", matrix_info(F))
    index_of_zero_rows_of_F = make_index_of_vec_n(n)
    csr_rows_set_nz_to_val(F, index_of_zero_rows_of_F, 0)
    # print("Finished..")

    # print("finish computing F...", matrix_info(F))

    # print("Finish.")
    # print("P^{T}:" )
    # print(P.T)
    # print("F:")
    # print(F)
    # print("nonzero of F")
    # nnzs = np.nonzero(F)
    # print(nonzeros_to_edges(nnzs, n))
    # print("multiply by -c")
    F *= -c

    # print("adding I to diagnol")
    F.setdiag(1)

    # print("eliminate_zeros...")
    F.eliminate_zeros()

    a = F

    inv_a = (1 - c) * inv(a).toarray()
    # inv_a =  inv(a).toarray()
    # print(inv_a.diagonal())

    # print(inv_a.sum(axis=1))

    # print("type of A: ", type(a))
    # print("shape of A", a.shape)
    # print("number of nonzeros of F: ", np.count_nonzero(c*F))
    # print("latex F")
    # print(matrix_to_latex_code(F))
    # print("generating I_nn")
    vec_I_nn = sp_vectorized_identity(n)
    # print("vec_I_nn:", matrix_info(vec_I_nn))
    b = vec_I_nn.todense()
    # print("shape of b: ", b.shape)

    # print("Solving the linear system...")
    if return_info == True:
        counter = gmres_counter()
        vec_S, code = sparse.linalg.gmres(a, b, callback=counter, \
                                          maxiter=ITERATIONS, tol=1e-50, restart=20, x0=b)
    else:
        vec_S, code = sparse.linalg.gmres(a, b)
    if code >= 0:
        # print("Finish computing simrank")
        if format_shape:
            S = vec_S.reshape((n, n))
        else:
            S = vec_S
        # print(matrix_info(S))
        if return_info:
            return (S, counter.results)
        else:
            return S
    else:
        print("Error occured while solving the linear system..")
        return -1
    # vec_S = sparse.linalg.spsolve(a, b)
    # S = vec_S.reshape((n,n))
    # return S


def sp_vectorized_identity(n):
    '''
    return a vectorized identity matrix of size n, so the vector is (n*n,1)
    '''
    rows = make_index_of_vec_n(n)
    cols = np.array([0] * n)
    data = np.array([1] * n)
    vec = sparse.csr_matrix((data, (rows, cols)), shape=(n * n, 1), dtype='int8')
    return vec


def nonzeros_to_edges(nnzs, size):
    '''
    nnzs: the array
    size: size of graph
    '''
    positions = list(zip(*nnzs))
    print("number of edges", len(positions))
    print("positions", positions)

    def one_component_to_pairs(y):
        c = y + 1
        a = (c - 1) // size + 1
        b = c - (a - 1) * size
        return (a, b)

    edges = list(map(lambda x: (one_component_to_pairs(x[0]), one_component_to_pairs(x[1])), positions))
    # generate graph nodes
    code = ''
    tmp = set()
    for edge in edges:
        a = edge[0]
        b = edge[1]
        tmp.add(a)
        tmp.add(b)
        code = code + str(b[0]) + str(b[1])
        code += " -> "
        code = code + str(a[0]) + str(a[1])
        code += ', \n'
    node_declare = ""
    for node in tmp:
        node_declare = node_declare + str(node[0]) + str(node[1]) + ' / ' + '$' + str(node[0]) + "|" + str(
            node[1]) + '$' + ', \n'
    print("tikz code: ")
    print(node_declare + code)
    return edges


def nonzeros_to_edges_for_nonsymmetric_situation(M, size):
    '''
    nnzs: the array of nonzeros positions
    size: size of the graph
    '''
    nnzs = np.nonzero(M)
    positions = list(zip(*nnzs))
    print("number of edges", len(positions))
    print("positions", positions)

    def one_component_to_pairs(y):
        count = 0
        for i in range(1, size + 1):
            for j in range(i, size + 1):
                count = count + 1
                if count == y + 1:
                    return (i, j)

    edges = list(map(lambda x: (one_component_to_pairs(x[0]), one_component_to_pairs(x[1]), M[x[0]][x[1]]), positions))
    # generate graph nodes
    code = ''
    tmp = set()
    for edge in edges:
        a = edge[0]
        b = edge[1]
        tmp.add(a)
        tmp.add(b)
        code = code + str(b[0]) + str(b[1])
        code += " -> [ "
        code += '''"$ '''
        code += str(edge[2])
        code += ''' $"'''
        code += ']'
        code = code + str(a[0]) + str(a[1])
        code += ', \n'
    node_declare = ""
    for node in tmp:
        node_declare = node_declare + str(node[0]) + str(node[1]) + ' / ' + '$' + str(node[0]) + "|" + str(
            node[1]) + '$' + ', \n'
    print("tikz code: ")
    print(node_declare + code)
    return edges


def matrix_to_latex_code(M):
    '''
    convert a matrix to latex form
    '''
    n = M.shape[0]
    s = ''
    for i in range(0, n):
        for j in range(0, n):
            s = s + ' & ' + str(M[i][j])
        s += "\\\\"
        s += '\n'
    return s


def test():
    # example_graph = []
    # for i in range(0,100):
    #     for j in range(0,100):
    #         example_graph.append((i,j))
    # m = adj_mat(example_graph)
    # print("m: ", matrix_info(m))
    # print(m.toarray().transpose())

    # s = simrank(m.T)
    # print(s)
    # print(s.sum(), m.shape[0]/(1-0.6))

    # for M in [A,B,C,D,E,H]:
    for M in [E, H]:
        m = adj_mat(M)
        # print("m: ", matrix_info(m))
        # print(m.toarray().transpose())
        s = false_sim(m.T.todense(), c=0.6)
        print(s)
        # l = cholesky(s)
        # print(np.dot(l, l.T))
        # # print(s.sum(), m.shape[0]/(1-0.6))
        # input("Press any keys to continue...")

    # s = symmetric_simrank(m.transpose(), return_info=True, format_shape=True, use_method="jacobi")
    # print("simrank, type of S: ", type(s))
    # print(s[0])
    # print(s[-1])

    # s = symmetric_simrank(m.transpose(), return_info=True, format_shape=True, use_method="gmres")
    # print("simrank, type of S: ", type(s))
    # print(s[0])
    # print(s[-1])
    # js = jacobi_simrank(m.transpose())
    # print("simrank, type of S: ", type(js))
    # print(js)

    # sym_s = symmetric_simrank(m.transpose())
    # print("reduced linear system simrank, type of s", type(sym_s))
    # print(sym_s)

    # ls_s = symmetric_simrank(m.transpose(), use_least_square=True)
    # print("reduced linear system simrank, type of s", type(ls_s))
    # print(ls_s)

    # iter_s = iter_simrank(sparse.csr_matrix(m), t=10)
    # print("iter simrank:, type of s", type(iter_s))
    # print(iter_s.todense())
    # sym_simrank(m.toarray())
    # false_sim(m)


if __name__ == "__main__":
    # g = nx.from_edgelist(A, create_using=nx.DiGraph())
    # g = nx.convert_node_labels_to_integers(g)
    # m = nx.to_scipy_sparse_matrix(g)
    # print(matrix_info(m))
    test()
