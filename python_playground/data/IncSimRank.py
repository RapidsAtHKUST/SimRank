# Tracking SimRank in Dynamic Graphs
import numpy as np
from itertools import chain, combinations, product
from scipy.sparse import csr_matrix, dok_matrix, dia_matrix
from data import *
from simrank import *
from numba import jit
from redis import StrictRedis
from persistentDict import PersistentDict
from LinkCutTree import DFG_Index

c = 0.6
NEW_DATA = ["odlis", "ca-GrQc", "p2p-Gnutella06", "ca-HepTh", "wiki-Vote", "web-NotreDame", "web-Stanford", \
            "web-BerkStan", "web-Google", "soc-pokec-relationships", "soc-LiveJournal1"]

LOCAL_PUSH_INDEX = './datasets/local_push_offline/'
DFG_INDEX = "./datasets/dfg_index/"
CPU_CORES = 10
PROCESS = cpu_count() - 4

# connect Redis
conn = StrictRedis()


def powerset(iterable):
    """
    powerset([1,2,3]) --> () (1,) (2,) (3,) (1,2) (1,3) (2,3) (1,2,3)
    """
    xs = list(iterable)
    # note we return an iterator rather than a list
    return chain.from_iterable(combinations(xs, n) for n in range(len(xs) + 1))


def examine(edgeList):
    m = adj_mat(edgeList)
    s = simrank(m.T)
    # print("Adj:")
    # print(m)
    a = s.sum()
    # print("simrank sum:", s.sum())
    return (a, m)


@jit
def get_row(A, i):
    '''
    get row i of csr matrix A
    '''
    row = A.indices[A.indptr[i]: A.indptr[i + 1]]
    return row


def P_name(data_name):
    return data_name + "_P.sqlite"


def R_name(data_name):
    return data_name + "_R.sqlite"


def dstack_product(x, y):
    return np.dstack(np.meshgrid(x, y)).reshape(-1, 2)


def load_local_push_index(data_name):
    '''
    load local push index given data name
    '''
    print("loading local push index", data_name)
    P = PersistentDict(LOCAL_PUSH_INDEX + data_name + "_P.json", flag="r", format="json")
    R = PersistentDict(LOCAL_PUSH_INDEX + data_name + "_R.json", flag="r", format="json")
    lpi = dict()
    lpi["P"] = P
    lpi["R"] = R
    return lpi


def local_push_simrank(A, indegrees, epsilon=0.01, delta=0.01, r_max=None, \
                       c=0.6, is_sync=False, data_name=None):
    '''
    the local push algorithm for all-pairs simrank: push all residuals until all below epsilon
    A: adjacency matrix, csr sparse matrix
    indegrees: index of in-degree of each node, 1darray
    r_max: the maximum residual
    to-do: make it parallel
    '''
    n = A.shape[0]
    m = A.nnz
    d = m / n
    if r_max is None:
        r_max = cal_rmax(d, epsilon, delta)
    print("threshold r", r_max)

    # the integer key for dict
    @jit
    def get_key(a1, a2):
        return a1 * n + a2

    # estimate = SyncableDict(redis=conn, key=data_name+"_P")
    # residual = SyncableDict(redis=conn, key=data_name+"_R")
    # estimate.clear()
    # residual.clear()
    # estimate = dict()
    # residual = dict()
    # if data_name is not None:
    #     off_line_file = shelve.open(LOCAL_PUSH_INDEX + data_name + ".shelve", protocol=4)
    with PersistentDict(LOCAL_PUSH_INDEX + data_name + "_P.json", flag="c", format="json") as estimate, \
            PersistentDict(LOCAL_PUSH_INDEX + data_name + "_R.json", flag="c", format="json") as residual:

        estimate.clear()
        residual.clear()
        for i in range(0, n):
            residual[get_key(i, i)] = 1
        Q = set([(i, i) for i in range(0, n)])  # use set to avoid dulplicate keys
        step = [0]
        current_sum = [0]  # the sum of current estimates

        @jit
        def push(a, b):
            """
            push the position a,b
            """
            k_ab = get_key(a, b)
            step[0] += 1
            r = residual.pop(k_ab, None)  # delete current key
            # r = residual[a,b]
            # residual[a,b] = 0
            # check whether key in the estimate
            estimate.setdefault(k_ab, 0)
            estimate[k_ab] += r
            current_sum[0] += r
            out_a = A.indices[A.indptr[a]: A.indptr[a + 1]]
            out_b = A.indices[A.indptr[b]: A.indptr[b + 1]]
            # print(len(out_a) * len(out_b))
            # for oa, ob in product(out_a, out_b):
            # for oa,ob in dstack_product(out_a, out_b):
            is_singleton_node = a == b
            for (oa, ob) in product(out_a, out_b):
                if oa == ob:  # don't push to singleton nodes
                    continue
                if oa > ob:  #
                    if is_singleton_node:  # residuals are from singletong nodes, only push to partial pairs
                        continue
                    else:
                        oa, ob = ob, oa
                k_oaob = get_key(oa, ob)
                indeg_a = indegrees[oa]
                indeg_b = indegrees[ob]
                total_in = indeg_a * indeg_b
                if total_in > 0:
                    inc = (c * r) / total_in
                    total_in = indeg_a * indeg_b
                    residual.setdefault(k_oaob, 0)
                    residual[k_oaob] += inc  # update residual value
                    # if is_sync: # the sync method
                    #     next_set.add((oa,ob))
                    # else:
                    if residual[k_oaob] > r_max:
                        Q.add((oa, ob))

        t1 = time.time()
        while len(Q) > 0:
            # check the memory
            # mem = psutil.virtual_memory()
            # mem_ratio = mem.available / mem.total
            i, j = Q.pop()
            # if step[0] % 1000 == 0:
            #     print(current_sum[0], len(Q), \
            #             "i,j: ", (i,j), "poped priority:",\
            #             residual[i,j], "step:", step)
            push(i, j)
        t2 = time.time()

        # save off-line index to disk
        print("total ", t2 - t1, "seconds")
        residual['time'] = t2 - t1
        print('origin sum', current_sum[0])
        print("sum", current_sum[0] * 2 - n)

        # sync to redis
        # print("syncing P")
        # estimate.sync()
        # print("syncing R")
        # residual.sync()

        # if off_line_file is not None:
        #     print("syncing to disk....")
        #     off_line_file["P"] = estimate
        #     off_line_file["R"] = residual
        #     off_line_file["time"] = t2-t1
        #     off_line_file["size"] = sys.getsizeof(estimate) + sys.getsizeof(residual)
        #     off_line_file.close()

        return True
        # if is_sync:
        #     # the traditional iterative method
        #     previous_set = Q # initialize the candidate set
        #     next_set = set()
        #     iterations = math.ceil(math.log(r_max, c))
        #     for t in range(0,iterations):
        #         print("iteration", t)
        #         while len(previous_set)>0:
        #             i,j = previous_set.pop()
        #             if step[0] % 1000 == 0:
        #                 print(current_sum[0], len(previous_set), "i,j: ", (i,j), "poped priority:", residual[i,j], "step:", step)
        #             push(i,j)
        #         previous_set, next_set = next_set, previous_set # swap the set, now next_set is empty again
        # else:


def cal_rmax(d, epsilon, delta, c=0.6):
    a = (1 - c) * (d ** 2) * (epsilon ** 2) / (c * (math.log(2 / delta)))
    return math.pow(a, 1 / 3)


def cal_N(r_max, epsilon=0.01, delta=0.01):
    N = (math.log(2) - math.log(delta)) * ((c * r_max) ** 2)
    N /= 2 * ((1 - c) ** 2) * (epsilon ** 2)
    N = math.ceil(N)
    return N


def one_way_graph(g):
    '''
    state the number of WCCs of g
    number_of_WCC: number of WCCs
    '''
    print("sampling one-way graph")
    sample_graph = nx.DiGraph()
    for node in g.nodes_iter():
        ins = g.in_edges(node)
        if len(ins) > 0:
            out = random.choice(ins)[0]
            edge = (node, out)
            sample_graph.add_edge(*edge)
    return sample_graph


def test_small():
    for M in [A, B, C, D, E, H]:
        m = adj_mat(M)
        mt = m.T
        s = local_push_simrank(csr_matrix(m), csr_matrix(mt).getnnz(1), r_max=0.0000001)
        n = m.shape[0]
        s_arr = dok_matrix((n, n), dtype=np.float64)
        for i in range(0, n):
            for j in range(i, n):
                s_arr[i, j] = s.setdefault((i, j), 0)
                s_arr[j, i] = s.setdefault((i, j), 0)
        print(s_arr.toarray())
        true_s = simrank(mt)
        print("*" * 10)
        print(true_s)
        print(s_arr.sum() - true_s.sum())
        print("-" * 20)
        # print("=" * 50)


def functional_graph_stat(data_name):
    '''
    (nodes, edges, number of w)
    '''
    g = load_graph(data_name)
    sg = one_way_graph(g)
    print("computing WCCs")
    number_of_WCC = nx.number_weakly_connected_components(sg)
    r = (g.number_of_nodes(), g.number_of_edges(), \
         number_of_WCC)
    print(r)
    return r


def merged_in_degree(at):
    '''
    at: transpose of ajacency matrix
    '''
    pass


def build_dfg(x):
    g, i, data_name = x
    with open(DFG_INDEX + data_name + "_" + str(i) + "_dfg.pkl", "wb") as fp:
        print("starting building ", i, "-th", data_name,)
        dfg = DFG_Index(g)
        print("dumping to disk")
        pickle.dump(dfg, fp)
        print("complete ", i, "th DFG-Index", "for", data_name)
    return True


def build_DFG_offline(data_name):
    '''
    build offline-functional graph index
    '''
    g = load_csr_g(data_name, is_tranpose=True)
    d = g.number_of_edges() / g.number_of_nodes()
    r_max = cal_rmax(d, 0.01, 0.01)
    N = cal_N(r_max)
    count = 0
    print("creating DFG index for ", data_name, "number:", N)
    with Pool(PROCESS, maxtasksperchild=1) as pool:
        t1 = time.time()
        dfgs = pool.map(build_dfg, [(g, i, data_name) for i in range(0, N)])
        t2 = time.time()
    print("complte, total", t2 - t1, "seconds")
    return t2 - t1


def build_offline_time():
    with PersistentDict("./local_push_time.json", format="json") as time_local_push:
        for data in NEW_DATA:
            print("processing", data)
            with PersistentDict(LOCAL_PUSH_INDEX + data + "_R.json", flag="r") as residual:
                time_local_push[data] = residual["time"]
    return


def test():
    # data = "web-Google"
    # data = "wiki-Vote"
    # data = "ca-GrQc"
    # with PersistentDict("./dfg_build_time.json", format="json") as time_dfg:
    #     for data in NEW_DATA[-1:]:
    #         t = build_DFG_offline(data)
    #         time_dfg[data] = t
    # data = NEW_DATA[-1]
    for data in NEW_DATA:
        a = load_sparse_csr(get_adj_file_path(data, is_tranpose=False))
        at = load_sparse_csr(get_adj_file_path(data, is_tranpose=True))
        d = a.nnz / a.shape[0]
        r_max = cal_rmax(d, 0.01, 0.01)
        N = cal_N(r_max)
        print(r_max, N)
    #     # s = local_push_simrank(a, at.getnnz(axis=1),\
    #     #         epsilon = 0.01, delta=0.01, \
    #     #         is_sync=False, data_name=data)
    # pprint(s.keys())
    # print(len(s))
    # print(a.shape[0] ** 2)
    # iter_simrank(a, epsilon=0.01)
    # d = dict()
    # for data in NEW_DATA:
    #     r = functional_graph_stat(data)
    #     d[data] = r
    # with open("./functional_graph_state.pkl", "wb") as f:
    #     pickle.dump(d, f)


if __name__ == "__main__":
    # build_offline_time()
    # test_small()
    test()
    # for data in NEW_DATA:
    #     a = load_sparse_csr(get_adj_file_path(data, is_tranpose=False))
    #     d = a.nnz / a.shape[0]
    #     print("degree", d)
    #     r = cal_rmax(d, 0.001, 0.01)
    #     print(r)
