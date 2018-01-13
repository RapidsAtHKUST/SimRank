# Monte carlo Method for single-pair simrank
from localPush import *


def truncated_random_walk(g, i, j, l, c=0.6):
    '''
    random walk for one coupled nodes
    return: 0 if they don't meet w_ithin l steps
            c^{step}
    g: graph
    i,j: starting nodes, and i != j
    l: the maximum length
    c: decay factor
    '''
    step = 0
    w_i = i  # state of walker i
    w_j = j  # state of walker j
    t = 0
    while t < l:
        if w_i == w_j:
            return c ** t
        else:
            # # move to next step
            # w_i = random.choice(g.in_edges(w_i))[0]
            # w_j = random.choice(g.in_edges(w_j))[0]
            # t += 1
            in_wi = g.in_edges(w_i)
            in_wj = g.in_edges(w_j)
            if len(in_wi) > 0 and len(in_wj) > 0:  # move to next step
                w_i = random.choice(in_wi)[0]
                w_j = random.choice(in_wj)[0]
                t += 1
            else:  # at least one has no in-neighbors
                return 0
    # faliure to meet w_ithin l steps
    return 0


def truncated_MC(g, pair, l=10, R=100, c=0.6):
    '''
    truncated random-walk based simrank
    g: the graph
    pair: the query pair (i,j)
    R: number of walkers
    l: the number of steps
    '''
    # print("truncated general random walk")
    i, j = pair
    simrank_score = 0
    if i == j:
        return 1
    for k in range(0, R):
        # repeate random walks for R times
        score = truncated_random_walk(g, i, j, l, c)
        simrank_score += score
    return simrank_score / R


def c_random_walk(g, i, j, c):
    '''
    random walk w_ith (1-c) probability
    return: 1 if meet 0 else
    '''
    w_i = i
    w_j = j
    while w_i != w_j:
        # stop w_ith 1-c
        if random.uniform(0, 1) > c:
            return 0
        else:
            # print("before", w_i, w_j)
            in_wi = g.in_edges(w_i)
            in_wj = g.in_edges(w_j)
            if len(in_wi) > 0 and len(in_wj) > 0:  # move to next step
                w_i = random.choice(in_wi)[0]
                w_j = random.choice(in_wj)[0]
                # print("after", w_i, w_j)
            else:  # at least one has no in-neighbors
                # print("has not inneighbor stop")
                return 0
    return 1


# @profile
def c_MC(g, pair, R=100, c=0.6):
    '''
    Monte carlo method w_ith (1-c) stop probability
    '''
    i, j = pair
    simrank_score = 0
    if i == j:
        return 1
    for k in range(0, R):
        result = c_random_walk(g, i, j, c)
        simrank_score += result
    # print(simrank_score)
    return simrank_score / R


def mem(data_name):
    data = data_name
    a = load_sparse_csr(get_adj_file_path(data, is_tranpose=False))
    # at = load_sparse_csr(get_adj_file_path(data, is_tranpose=True))
    g = nx.from_scipy_sparse_matrix(a, create_using=nx.DiGraph())
    # load groaud truth
    # simrank_a = np.load("./datasets/ground_truth_SimRank/ca-GrQc.npy")
    # asso_D = np.load(get_D_path(data_name))
    pairs = queryGen()[0:3]
    for pair in pairs:
        # truth = simrank_a[pair]
        # truncated = truncated_MC(g, pair, R= 100)
        # c_stop = c_MC(g, pair, R=25000)
        # linear_iter = linear_single_pair(g, pair, asso_D)
        local_push = synchron_local_push_max_r(g, pair, is_async=True, error=0.0001)
        # print(truth, truncated , c_stop , linear_iter, local_push)
        print(local_push)


def test():
    data = "ca-GrQc"
    a = load_sparse_csr(get_adj_file_path(data, is_tranpose=False))
    at = load_sparse_csr(get_adj_file_path(data, is_tranpose=True))
    g = nx.from_scipy_sparse_matrix(a, create_using=nx.DiGraph())
    # load groaud truth
    simrank_a = np.load("./datasets/ground_truth_SimRank/ca-GrQc.npy")
    asso_D = np.load(get_D_path("ca-GrQc"))
    pairs = queryGen()[0:50]
    for pair in pairs:
        truth = simrank_a[pair]
        truncated = truncated_MC(g, pair, R=100)
        c_stop = c_MC(g, pair, R=100)
        linear_iter = linear_single_pair(g, pair, asso_D)
        local_push = synchron_local_push_max_r(g, pair, is_async=True, error=0.0001)
        print(truth, truncated, c_stop, linear_iter, local_push)


if __name__ == '__main__':
    test()
    # DATA = ["odlis", "ca-GrQc", "p2p-Gnutella06", "ca-HepTh", "wiki-Vote"]
    # for d in DATA:
    #     mem(d)
    #     input("pressing any key to continue...")
