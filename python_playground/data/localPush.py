# local push algorithm for single-pair
from MC_simrank import *

DIRECTORY = "./datasets/experiments/"
DIR = DIRECTORY + "push_residuals/"


# @profile
def synchron_local_push_max_r(g, pair, error=0.001, c=0.6, is_async=False, max_iter=30000, \
                              residual_recorder=None):
    '''
    the maximum residual is less than error
    g: the input graph
    pair: the query pair
    error: the termination criterion, usually the max error
    max_iter: max length of BFS depth
    residual_recorder: a list that records the residual sum
    '''
    print(pair)
    i, j = pair
    if (i == j):
        return 1
    n = g.number_of_nodes()
    p = dok_matrix((n, n), dtype=np.float32)
    r = dok_matrix((n, n), dtype=np.float32)
    r[i, j] = 1
    residual_sum = [1]
    simrank_score = [0]
    max_residual = []
    candidate = PrioritySet()
    # heappush(max_residual, (-residual_sum[0], pair)) # init the heapq
    candidate.add(pair, -residual_sum[0])
    iterations = 0

    def push(x, y):
        '''
        push the residual of (x,y)
        '''
        if residual_recorder is not None:
            residual_recorder.append(residual_sum[0])  # recorder the residual
        tmp = r[x, y]
        p[x, y] += tmp
        r[x, y] = 0
        # update residual sum
        residual_sum[0] -= tmp
        # in neighbors of x and y
        if (x == y):
            # also a dead node, no more operation needed
            simrank_score[0] += tmp
            return
        inx = g.in_edges(x)
        iny = g.in_edges(y)
        n_inneighbors = len(inx) * len(iny)
        # print(x,y,n_inneighbors, residual_sum[0], len(max_residual))
        # print(simrank_score[0])
        if n_inneighbors > 0:
            vol = (c * tmp / n_inneighbors)
            for a in inx:
                for b in iny:
                    a0, b0 = a[0], b[0]
                    residual_sum[0] += vol
                    r[a0, b0] += vol
                    # update heap
                    if is_async:
                        if r[a0, b0] > 0 and (a0, b0) not in candidate.set:
                            # candidate.add((a0,b0))
                            candidate.add((a0, b0), -r[a0, b0])

    if not is_async:
        iterations = 0
        while residual_sum[0] > error and iterations < max_iter:
            iterations += 1
            if iterations > max_iter:  # exceed the maximum depth of BFS
                break
            all_nnzs = list(r.keys())
            for k1, k2 in all_nnzs:
                if r[k1, k2] > 0:
                    push(k1, k2)
    elif is_async:
        # while r.nnz > 0 and len(candidate) > 0:
        while residual_sum[0] > error:
            iterations += 1
            # if iterations % 1000 == 0:
            #     print(simrank_score[0])
            k1, k2 = candidate.get()
            push(k1, k2)
    return simrank_score[0]


def basic_syn_local_push(g, pair, error=0.0005, c=0.6, epsilon=0.01, is_async=False, max_iter=30000, \
                         residual_recorder=None):
    '''
    the basic synchronic local push algorithm, iterates to a fixed point.
    the maximum residual is less than error
    g: the input graph
    pair: the query pair
    error: the termination criterion, usually the max error
    max_iter: max length of BFS depth
    residual_recorder: a list that records the residual sum
    '''
    print(pair)
    i, j = pair
    if (i == j):
        return 1
    n = g.number_of_nodes()
    p = dok_matrix((n, n), dtype=np.float32)
    r = dok_matrix((n, n), dtype=np.float32)
    r[i, j] = 1
    residual_sum = [1]
    simrank_score = [0]
    max_residual = []
    previous_set = set()  # the set to hold the current iteration
    next_set = set()  # the set to hold the next iteration
    # heappush(max_residual, (-residual_sum[0], pair)) # init the heapq
    # candidate.add(pair, -residual_sum[0])
    previous_set.add(pair)  # pair is a tuple
    count = 0
    iterations = math.ceil(math.log(epsilon, c))

    def push(x, y):
        '''
        push the residual of (x,y)
        '''
        if residual_recorder is not None:
            residual_recorder.append(residual_sum[0])  # recorder the residual
        tmp = r[x, y]
        p[x, y] += tmp
        r[x, y] = 0
        # update residual sum
        residual_sum[0] -= tmp
        # in neighbors of x and y
        if (x == y):
            # also a dead node, no more operation needed
            simrank_score[0] += tmp
            return
        # print(simrank_score[0])
        inx = g.in_edges(x)
        iny = g.in_edges(y)
        n_inneighbors = len(inx) * len(iny)
        # print(x,y,n_inneighbors, residual_sum[0], len(max_residual))
        if n_inneighbors > 0:
            vol = (c * tmp / n_inneighbors)
            for a in inx:
                for b in iny:
                    a0, b0 = a[0], b[0]
                    residual_sum[0] += vol
                    r[a0, b0] += vol
                    # update the set of next round
                    if r[a0, b0] > error:
                        next_set.add((a0, b0))

    while count < iterations:
        count += 1
        ## pull from previous_set to BFS
        while len(previous_set) > 0:
            # clear out the for the elements of next round
            # print(count, len(previous_set))
            k1, k2 = previous_set.pop()
            push(k1, k2)
        previous_set, next_set = next_set, previous_set  # swap the role: previous_set now have elements, while next set is empty
    return simrank_score[0]


def synchron_local_push(g, pair, error=0.01, c=0.6, is_async=False, max_iter=30000, residual_recorder=None, \
                        predict_recorder=None):
    '''
    g: the input graph
    pair: the query pair
    error: the termination criterion, usually the max error
    max_iter: max length of BFS depth
    '''
    print(pair)
    i, j = pair
    if (i == j):
        return 1
    n = g.number_of_nodes()
    p = dok_matrix((n, n), dtype=np.float32)
    r = dok_matrix((n, n), dtype=np.float32)
    r[i, j] = 1
    residual_sum = [1]
    simrank_score = [0]
    max_residual = []
    candidate = set()
    heappush(max_residual, (-residual_sum[0], pair))  # init the heapq
    candidate.add(pair)
    iterations = 0

    def push(x, y):
        '''
        push the residual of (x,y)
        '''
        nonlocal
        iterations
        iterations += 1
        if iterations > max_iter:
            return
        # print(iterations, residual_sum[0], simrank_score[0])
        if residual_recorder is not None:
            residual_recorder.append(residual_sum[0])
        if predict_recorder is not None:
            predict_recorder.append(simrank_score[0])
        tmp = r[x, y]
        p[x, y] += tmp
        r[x, y] = 0
        # update residual sum
        residual_sum[0] -= tmp
        # in neighbors of x and y
        if (x == y):
            # also a dead node, no more operation needed
            simrank_score[0] += tmp
            return
        inx = g.in_edges(x)
        iny = g.in_edges(y)
        n_inneighbors = len(inx) * len(iny)
        # print(x,y,n_inneighbors, residual_sum[0], len(max_residual))
        if n_inneighbors > 0:
            vol = (c * tmp / n_inneighbors)
            for a in inx:
                for b in iny:
                    a0, b0 = a[0], b[0]
                    residual_sum[0] += vol
                    r[a0, b0] += vol
                    # update heap
                    if is_async:
                        if len(max_residual) == 0:
                            heappush(max_residual, (-r[a0, b0], (a0, b0)))
                        if r[a0, b0] > -max_residual[0][0]:  # or larger than current maximum
                            heapreplace(max_residual, (-r[a0, b0], (a0, b0)))
                            # heappush(max_residual, (-r[a0, b0], (a0,b0)))
                        # if r[a0,b0] > residual_sum[0] / (r.nnz + 2):
                        #     candidate.add((a0,b0))

    if not is_async:
        while residual_sum[0] > error and iterations < max_iter:
            all_nnzs = list(r.keys())
            for k1, k2 in all_nnzs:
                if r[k1, k2] > 0:
                    push(k1, k2)
    elif is_async:
        while r.nnz > 0 and residual_sum[0] > error and iterations < max_iter:
            print(iterations, residual_sum[0])
            if len(max_residual) == 0:
                for s, t in r.keys():
                    if r[s, t] > residual_sum[0] / (r.nnz + 2):
                        heappush(max_residual, (-r[s, t], (s, t)))
            current_residual, (k1, k2) = heappop(max_residual)
            push(k1, k2)
    return simrank_score[0]


def compare_local_syn_asy():
    a = load_sparse_csr("./datasets/adj/ca-GrQc.npz")
    g = nx.from_scipy_sparse_matrix(a, create_using=nx.DiGraph())
    # load groaud truth
    simrank_a = np.load("./datasets/ground_truth_SimRank/ca-GrQc.npy")
    num = 0
    pairs = queryGen()[0:200]
    print(len(pairs))
    for pair in pairs:
        if num > 4:
            break
        else:
            truth = simrank_a[pair]
            if truth > 0.1:
                num += 1
                file_name = DIR + str(pair[0]) + str(pair[1]) + "sync_async_localpush"
                result = dict()
                result["query_pair"] = pair
                result["ground_truth"] = truth
                # truncated = truncated_MC(g, pair, R= 1000)
                # c_stop = c_MC(g, pair, R=1000)
                print("pair", pair, "ground truth", truth)
                asy_recorder = []
                syn_recorder = []
                asy_p = []
                syn_p = []
                syn_local_push = synchron_local_push(g, pair, is_async=False, residual_recorder=syn_recorder, \
                                                     predict_recorder=syn_p)
                asyc_local_push = synchron_local_push(g, pair, is_async=True, residual_recorder=asy_recorder, \
                                                      predict_recorder=asy_p)
                result["asyc_residuals"] = asy_recorder
                result["sync_residuals"] = syn_recorder
                result["asyc_predict"] = asy_p
                result["sync_predict"] = syn_p
                result["asyc_r"] = asyc_local_push
                result["sync_r"] = syn_local_push
                with open(file_name + ".pkl", "wb") as f:
                    pickle.dump(result, f)
                print(syn_local_push)


def test():
    a = load_sparse_csr("./datasets/adj/ca-GrQc.npz")
    g = nx.from_scipy_sparse_matrix(a, create_using=nx.DiGraph())
    # load groaud truth
    simrank_a = np.load("./datasets/ground_truth_SimRank/ca-GrQc.npy")
    print(simrank_a.sum(), a.shape[0] / (1 - 0.6))
    pairs = queryGen()[0:200:50]
    print(len(pairs))
    for pair in pairs:
        truth = simrank_a[pair]
        truncated = truncated_MC(g, pair, R=1000)
        c_stop = c_MC(g, pair, R=1000)
        # print("pair", pair, "ground truth", truth)
        asy_recorder = []
        syn_recorder = []
        local_push = basic_syn_local_push(g, pair)
        # syn_local_push = synchron_local_push(g, pair, is_async=True, residual_recorder=asy_recorder)
        # syn_local_push = synchron_local_push_max_r(g, pair, is_async=True)
        # print(truth, truncated, c_stop, )
        print("truth", truth, local_push, c_stop, truncated)
        input("Press any keys to continue...")
        # input("Press any keys to continue...")
        # print(truth, truncated, c_stop)


if __name__ == '__main__':
    test()
    # compare_local_syn_asy()
