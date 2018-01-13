import heapq
import math
import time
from multiprocessing import Pool, freeze_support, cpu_count

import numba
import numpy as np
import scipy
from scipy import sparse

# set the printing config
np.set_printoptions(precision=3)

# set the core number
PROCESS = cpu_count() - 4


# set correspoing rows to zero
def csr_row_set_nz_to_val(csr, row, value=0):
    """
    index of row starts with 0
    Set all nonzero elements (elements currently in the sparsity pattern)
    to the given value. Useful to set to 0 mostly.
    """
    if not isinstance(csr, scipy.sparse.csr_matrix):
        raise ValueError('Matrix given must be of CSR format.')
    csr.data[csr.indptr[row]:csr.indptr[row + 1]] = value


def csr_rows_set_nz_to_val(csr, rows, value=0):
    for row in rows:
        csr_row_set_nz_to_val(csr, row)
    # if value == 0:
    #     csr.eliminate_zeros()


class PrioritySet(object):
    def __init__(self):
        self.heap = []
        self.set = set()

    def add(self, d, pri):
        if not d in self.set:
            heapq.heappush(self.heap, (pri, d))
            self.set.add(d)

    def get(self):
        pri, d = heapq.heappop(self.heap)
        self.set.remove(d)
        return d

    def __len__(self):
        return len(self.set)


def getQ(n):
    '''
    generate Q for symmetric kronecker product
    n: the size of the symmetric matrix
    '''
    columns = n * n
    rows = int((n + 1) * n / 2)
    Q = np.zeros((rows, columns))
    size = n
    x = 1  # index for rows, i,j is for block by block
    for i in range(1, n + 1):
        # block by block
        for j in range(1, 1 + size):
            if j == 1:
                # it is a identity element
                Q[x - 1][i + (i - 1) * n - 1] = 1
            else:
                jj = i + j - 1
                # print(i,jj)
                value = math.pow(2, 0.5) / 2
                Q[x - 1][(i - 1) * n + jj - 1] = value
                Q[x - 1][(jj - 1) * n + i - 1] = value
            x = x + 1
        size = size - 1
    # print(np.around(Q, decimals=1))
    return Q


# I/O of sparse matrix
def save_sparse_csr(filename, array):
    np.savez(filename, data=array.data, indices=array.indices,
             indptr=array.indptr, shape=array.shape)


def load_sparse_csr(filename):
    print("loading " + filename)
    loader = np.load(filename)
    print("loading complete")
    return sparse.csr_matrix((loader['data'], loader['indices'], loader['indptr']),
                             shape=loader['shape'])


def make_index_of_vec_n(n):
    '''
    n: for I(n)
    vectorize a identity matrix of size n
    '''
    return np.array([(i - 1) * n + i - 1 for i in range(1, n + 1)])


# string of sparse matrix infomation
def matrix_info(M):
    '''
    print information of a matrix
    '''
    s = "type: " + str(type(M)) + " shape: " + str(M.shape) + " memory: " + str(
        M.data.nbytes) + " bytes  " + " dtype: " + str(M.dtype) + " nnz: " + str(M.nnz)
    return s


# CSR kronecker product
@numba.jit
def csr_kron_F(a, b):
    '''
    usage: a is A.T
    compute csr format of bool-type kronecker product
    a: a csr format sparse matrix, dtype = bool
    b: a csr format sparse matrix, dtype = bool
    c: a csr format sparse matrix, dtype = float32, the value is the 1/I(v) for node v.
    Notice: F has diagonal elements to make further operation easy
    '''
    p, q = a.shape
    r, s = b.shape
    m = p * r
    n = q * s
    count = 0  # cursor for indices and data update
    indptr = np.zeros(m + 1, dtype="int")  # init indprt
    indices = np.zeros(a.nnz * b.nnz + p * r, dtype="int")  # init row index
    data = np.zeros(a.nnz * b.nnz + p * r,
                    dtype="float32")  # initialize data, with float64, there are p*r diagonal elements
    for i in range(0, p):
        # print("i: ", i)
        a_start = a.indptr[i]
        a_end = a.indptr[i + 1]
        a_nnz = a_end - a_start
        a_indice = a.indices[a_start:a_end]
        for j in range(0, r):
            row_number = i * p + j
            # print(row_number)
            b_start = b.indptr[j]
            b_end = b.indptr[j + 1]
            b_indice = b.indices[b_start:b_end]
            b_nnz = b_end - b_start
            number_of_nnz = a_nnz * b_nnz
            if number_of_nnz == 0:
                normalized_value = 1
            else:
                normalized_value = 1.0 / number_of_nnz  # value of this row
            # update intprt
            indptr[row_number + 1] = indptr[row_number] + number_of_nnz + 1  # 1 is for the diagonal element
            # update the data with in-order of this row
            data[indptr[row_number]:indptr[row_number + 1]] = normalized_value
            # udpate column indices and the data array in a numpy manner
            a_kron = np.repeat(s * a_indice, b_indice.size)
            b_kron = np.tile(b_indice, a_indice.size)
            c_indices = a_kron + b_kron
            indices[indptr[row_number]:indptr[row_number + 1]] = np.append(c_indices, np.array(
                [row_number]))  # row_number is for diagonal element
    # print("computation complete, constructing csr matrix")
    csr_result = sparse.csr_matrix((data, indices, indptr), shape=(m, n), dtype="float32", copy=False)
    return csr_result


@numba.jit
def get_row_number(x, y, n):
    '''
    a tool function for symmetric kronecker product
    get row numyer for index x xnd y
    x: stxrts from 0
    y: stxrts from 0
    '''
    if x > y:
        p = y + 1
        q = x + 1
    else:
        p = x + 1
        q = y + 1
    # print(p,q,type(p))
    return (p - 1) * n - int((p - 1) * (p - 2) / 2) + q - p


@numba.jit
def symmetric_csr_kron(AT):
    '''
    The difference with original kron is that F_s's diagonal element is correct, while F is initialized with 1
    compute F_s, given A.T
    return: F_s
    '''
    n = AT.shape[0]
    m = AT.nnz
    nnz_Fm = int(m * (m + 1) / 2)  # nnz for edges
    nnz_Fn = int(n * (n + 1) / 2)  # nnz for nodes, also for rows number
    total_nnz = nnz_Fm + nnz_Fn
    indptr = np.zeros(nnz_Fn + 1, dtype="int")
    indices = np.zeros(total_nnz, dtype="int")  # we have extra diagonal element
    data = np.zeros(total_nnz, dtype="float32")
    count = 0
    multiplier = math.sqrt(2)
    for i in np.arange(0, n):
        a_start = AT.indptr[i]
        a_end = AT.indptr[i + 1]
        a_nnz = a_end - a_start
        a_indice = AT.indices[a_start:a_end]
        print("i", i)
        for j in np.arange(i, n):
            row_number = get_row_number(i, j, n)  # index starts from 0
            # print(row_number)
            b_start = AT.indptr[j]
            b_end = AT.indptr[j + 1]
            b_indice = AT.indices[b_start:b_end]
            b_nnz = b_end - b_start
            number_of_nnz = a_nnz * b_nnz
            if number_of_nnz == 0:
                normalized_value = 0
            else:
                normalized_value = 1.0 / number_of_nnz  # value of this row
            # make a dictionary to hold values, and construct the row from the dictionary
            row_dok = dict()  # the nnz to current row
            row_dok.setdefault(row_number, 1)  # set the diagonal element
            # add the diagonal element, initial with 0
            row_dok[row_number] = 0
            if i != j and number_of_nnz > 0:
                # the case we need compute kronecker product
                # we need to calculate kronecker product
                print("constructing the dictionary")
                for k in np.nditer(a_indice):
                    for l in np.nditer(b_indice):
                        column_index = get_row_number(k, l, n)
                        row_dok.setdefault(column_index, 0)
                        if k != l:
                            row_dok[column_index] += normalized_value
                        else:  # the case k == l
                            row_dok[column_index] += multiplier * normalized_value
            nnz_of_this_row = len(row_dok)
            indptr[row_number + 1] = indptr[row_number] + nnz_of_this_row
            # build the indices and data
            print("updating the indices and data array")
            for index, value in row_dok.items():
                indices[count] = index
                data[count] = value
                count += 1
    print("indprt", len(indptr))
    print("indices", len(indices))
    print("data", len(data))
    print("computation complete, constructing csr matrix")
    csr_result = sparse.csr_matrix((data, indices, indptr), shape=(nnz_Fn, nnz_Fn), dtype="float32", copy=False)
    return csr_result


AT = None  # used for shared between processes


def parallel_symmetric_csr_kron(A_transpose):
    '''
    multiprocessing based symmetric kronecker product
    A_transpose: A.T, transpose of a adjacency matrix
    '''
    global AT
    AT = A_transpose
    n = AT.shape[0]
    m = AT.nnz
    # initialize csr three arrays
    nnz_Fm = int(m * (m + 1) / 2)  # nnz for edges
    nnz_Fn = int(n * (n + 1) / 2)  # nnz for nodes, also for rows number
    total_nnz = nnz_Fm + nnz_Fn
    indptr = np.zeros(nnz_Fn + 1, dtype="int")
    indices = np.zeros(total_nnz, dtype="int")  # we have extra diagonal element
    data = np.zeros(total_nnz, dtype="float32")

    with Pool(PROCESS) as pool:
        print("generating tasks")
        task_list = [(i, j, n) for i in range(0, n) for j in range(i, n)]
        print("cpu cores: ", PROCESS)
        print("calculating rows...")
        chunk_length = max(nnz_Fn // (50 * PROCESS), 1)
        print("chunksize", chunk_length)
        rows = pool.map_async(generate_csr_row, task_list, chunksize=chunk_length)  # keep orders
        print("begin tasks...")
        while True:
            if rows.ready():
                break
            else:
                print("Remaining task ", rows._number_left)
                time.sleep(7)  # wait for 7 seconds
        results_rows = rows.get()  # now task complete
    print("sorting the results")
    results_rows.sort(key=lambda x: x[0])
    print("filling in the three arrays of CSR...")
    for row in results_rows:  # now rows are already sorted
        row_number, row_nnz, row_indices, row_data = row
        indptr[row_number + 1] = indptr[row_number] + row_nnz
        indices[indptr[row_number]:indptr[row_number + 1]] = row_indices
        data[indptr[row_number]:indptr[row_number + 1]] = row_data

    print("computation complete, constructing csr matrix")
    csr_result = sparse.csr_matrix((data, indices, indptr), shape=(nnz_Fn, nnz_Fn), dtype="float32", copy=False)
    return csr_result


def generate_csr_row(parameters):
    '''
    parameters: (i,j,n)
    multiprocessing method for indivadual row generation for symmetric kronecker product
    return: (row_number, nnz, indices, data)
    nnz: int
    indices: numpy array
    data: numpy array
    row_number: int
    '''
    i, j, n = parameters
    row_number = get_row_number(i, j, n)
    # initialize values
    multiplier = math.sqrt(2)
    a_start = AT.indptr[i]
    a_end = AT.indptr[i + 1]
    a_nnz = a_end - a_start
    a_indice = AT.indices[a_start:a_end]
    b_start = AT.indptr[j]
    b_end = AT.indptr[j + 1]
    b_indice = AT.indices[b_start:b_end]
    b_nnz = b_end - b_start
    number_of_nnz = a_nnz * b_nnz
    if number_of_nnz == 0:
        normalized_value = 0
    else:
        normalized_value = 1.0 / number_of_nnz  # value of this row
    row_dok = dict()  # the nnz to current row
    row_dok.setdefault(row_number, 1)  # set the diagonal element
    # add the diagonal element, initial with 0
    row_dok[row_number] = 0
    if i != j and number_of_nnz > 0:
        # the case we need compute kronecker product
        # we need to calculate kronecker product
        for k in np.nditer(a_indice):
            for l in np.nditer(b_indice):
                column_index = get_row_number(k, l, n)
                row_dok.setdefault(column_index, 0)
                if k != l:
                    row_dok[column_index] += normalized_value
                else:  # the case k == l
                    row_dok[column_index] += multiplier * normalized_value
    nnz = len(row_dok)
    indices = np.zeros(nnz, dtype="int")
    data = np.zeros(nnz, dtype="float32")
    count = 0
    for index, value in row_dok.items():
        indices[count] = index
        data[count] = value
        count += 1
    # return values
    # print("finish task:", i, " ", j)
    return (row_number, nnz, indices, data)


def test():
    import simrank
    AT = np.array([
        [0, 1, 1, 0],
        [0, 0, 1, 0],
        [0, 0, 0, 1],
        [1, 0, 0, 0]
    ])
    a = simrank.adj_mat(simrank.E)
    print("a: ",)
    print(AT)
    b = parallel_symmetric_csr_kron(sparse.csr_matrix(AT))
    print(matrix_info(b))
    # print(b.astype("bool").astype("int8").toarray())
    print(np.around(b.toarray(), 2))
    # print(b.toarray())


if __name__ == '__main__':
    freeze_support()
    test()
    # a = sparse.eye(3, dtype="bool").tocsr()
    # b = csr_kron(a,a)
    # print("b: ", matrix_info(b))
    # print(b.astype("int8").toarray())
