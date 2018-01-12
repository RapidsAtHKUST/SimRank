import math

import networkx as nx
from pympler import asizeof
from reads.readsd import readsd

from MC_simrank import truncated_MC
from data import *


def cal_reads_N(epsilon=0.01, delta=0.01, c=0.6):
    N = math.ceil((c ** 6) * (math.log(delta) - math.log(2)) / (-2 * (epsilon ** 2)))
    return N


n = 3133
c = 0.6
t = 10
r = cal_reads_N()


def test():
    gName = "./datasets/hp.txt"
    r = 1000
    n = 3133
    c = 0.6
    t = 10
    r_index = readsd(gName, n, r, c, t)
    readsd_rs = [0.0] * n
    cmc_rs = [0] * n
    g = nx.read_edgelist(gName, create_using=nx.DiGraph(), nodetype=int)
    print(type(r_index))
    print("index size", asizeof.asizeof(r_index))
    for i in range(0, n):
        readsd_rs[i] = r_index.queryOne(0, i)
        cmc_rs[i] = truncated_MC(g, (0, i), R=r)
        print(i, readsd_rs[i], cmc_rs[i])
    print("inserting...")
    r_index.insEdge(3, 7)
    r_index.insEdge(5, 55)
    r_index.insEdge(6, 666)
    print("finish...")
    return


if __name__ == '__main__':
    # test()
    print(r)
    # with PersistentDict("./readsd_build_time.json", format="json") as p:
    #     for data_name in DATA:
    #         t1 = time.time()
    #         a = load_sparse_csr(get_adj_file_path(data_name, is_tranpose=False))
    #         n = a.shape[0]
    #         print("building index for ", data_name, "numbers: ", r)
    #         r_index = readsd(get_edge_list_path(data_name), n, r, c, t)
    #         t2 = time.time()
    #         print("building complete")
    #         p[data_name] = t2-t1
