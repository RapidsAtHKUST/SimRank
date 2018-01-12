'''
The Link-Cut Tree data structure
'''
from numba import jit
from scipy.sparse.csr import csr_matrix

from data import *


# define the Splay tree
class Node:
    def __init__(self, p=None, pp=None, l=None, r=None, \
                 size=1, identifier=None, label=None):
        self.p = p  # parent
        self.pp = pp  # path parent
        self.l = l  # left child
        self.r = r  # right child
        self.size = size  # size of subtree, including itself
        self.label = label
        self._id = identifier

    def left_size(self):
        '''
        size of left subtree
        '''
        if self.l is None:
            return 0
        else:
            return self.l.size

    def right_size(self):
        '''
        size of right subtree
        '''
        if self.r is None:
            return 0
        else:
            return self.r.size

    def is_root(self):
        '''
        whether is the root of splay tree
        '''
        return (self.p == None) or (self.p.l != self and \
                                    self.p.r != self)


def update(x):
    '''
    update the value of node x
    '''
    x.size = 1
    if x.l is not None:
        x.size += x.l.size
    if x.r is not None:
        x.size += x.r.size
    return x


def rotate_right(x):
    '''
    rotate x rightly
    '''
    y = x.p
    z = y.p

    y.l = x.r
    if y.l is not None:
        y.l.p = y
    x.r = y
    y.p = x

    x.p = z
    if x.p is not None:
        if y == z.l:
            z.l = x
        else:
            z.r = x
    x.pp = y.pp
    y.pp = None
    update(y)


def rotate_left(x):
    y = x.p
    z = y.p

    y.r = x.l
    if y.r is not None:
        y.r.p = y
    x.l = y
    y.p = x

    x.p = z
    if x.p is not None:
        if y == z.l:
            z.l = x
        else:
            z.r = x
    x.pp = y.pp
    y.pp = None
    update(y)


def splay(x):
    '''
    splay x to the root
    '''
    while x.p is not None:
        y = x.p
        if y.p is None:
            if x == y.l:
                rotate_right(x)
            else:
                rotate_left(x)
        else:
            z = y.p
            if y == z.l:
                if x == y.l:
                    rotate_right(y)
                    rotate_right(x)
                else:
                    rotate_left(x)
                    rotate_right(x)
            else:
                if x == y.r:
                    rotate_left(y)
                    rotate_left(x)
                else:
                    rotate_right(x)
                    rotate_left(x)
    update(x)
    return


def find_kth_large(root, k):
    '''
    find the kth largest number of a splay tree
    k: starting from 1
    '''
    if k < 1 or k > root.size:
        print("k: ", k, "is out of range")
        return None
    r_size = root.right_size()
    if k == r_size + 1:
        return root
    elif k > 1 + r_size:  # in the left subtree
        return find_kth_large(root.l, k - r_size - 1)
    else:  # in the right subtree
        return find_kth_large(root.r, k)


# Link-cut tree operations
def access(x):
    splay(x)
    if x.r is not None:
        x.r.pp = x
        x.r.p = None
        x.r = None
        update(x)
    last = x
    while x.pp is not None:
        y = x.pp
        last = y
        splay(y)
        if y.r is not None:
            y.r.pp = y
            y.r.p = None
        y.r = x
        x.p = y
        x.pp = None
        update(y)
        splay(x)
    return last


def find_root(x):
    access(x)
    while x.l is not None:
        x = x.l
    splay(x)
    return x


def is_root(x):
    '''
    whether x is the root of the link-cut tree
    '''
    y = find_root(x)
    return x == y


def cut(x):
    access(x)
    x.l.p = None
    x.l = None
    update(x)


def link(x, y):
    # link x as y.subtree
    access(x)
    access(y)
    x.l = y
    y.p = x
    update(x)


def depth(x):
    access(x)
    return x.size - 1


def lca(x, y):
    rx = find_root(x)
    ry = find_root(y)
    if rx != ry:
        print("have no lca")
        return None
    else:
        access(x)
        last = access(y)
        return last


def kth_ancestor(x, k):
    '''
    k-th ancestor of x,
    k can be 0 when it is x itself
    '''
    # print("querying kth ancestor")
    access(x)
    return find_kth_large(x, k + 1)


# method binding for csr matrix shape graph
@jit
def get_row(A, i):
    '''
    get row i of csr matrix A
    '''
    row = A.indices[A.indptr[i]: A.indptr[i + 1]]
    return row


@jit
def number_of_nodes(self):
    return self.shape[0]


def number_of_edges(self):
    return self.nnz


@jit
def nodes(self):
    n = self.number_of_nodes()
    return list(range(0, n))


@jit
def in_degree(self, a):
    return self.indptr[a + 1] - self.indptr[a]


@jit
def in_edges(self, a):
    in_neighbors = get_row(self, a)
    a_s = np.repeat(a, len(in_neighbors))
    return list(zip(in_neighbors, a_s))


csr_matrix.number_of_nodes = number_of_nodes
csr_matrix.number_of_edges = number_of_edges
csr_matrix.nodes = nodes
csr_matrix.in_edges = in_edges
csr_matrix.in_degree = in_degree


# The DFG-Index
class DFG_Index:
    '''
    sampled one-way graph from g, support dynamic update
    '''

    @jit
    def __init__(self, g):
        '''
        build one-way graph from g
        label attribute: starting node of current path
        adj_T: the sparse transpose matrix of graph
        '''
        # init the
        self.root_ptr = dict()  # key is root node identifier
        self.nodes = dict()  # key nodes id
        current_color = None
        n = g.number_of_nodes()
        all_nodes = set(g.nodes())  # list of all vertices

        # start exploring the graph
        while len(all_nodes) > 0:
            # start a new path
            a = all_nodes.pop()
            current_color = a
            self.nodes[a] = Node(label=current_color, \
                                 identifier=a)
            # expand current tree
            if g.in_degree(a) > 0:
                b = random.choice(g.in_edges(a))[0]
            else:
                continue
            while b not in self.nodes:
                self.nodes[b] = Node( \
                    label=current_color, \
                    identifier=b)
                # print("linking ", a , "to", b)
                link(self.nodes[a], self.nodes[b])
                a = b  # a is considered, we can remove it
                all_nodes.remove(a)
                if g.in_degree(a) > 0:  # a has in-neighbors
                    b = random.choice(g.in_edges(a))[0]
                else:  # a has no in-neighbor
                    break
            else:  # executed when while loop exit normally
                # now we meet a existing node
                if self.nodes[b].label == current_color:
                    # a new WCC is created, a new cycle
                    self.root_ptr[a] = b
                else:
                    # link to an existing WCC
                    # print("linking ", a , "to", b)
                    link(self.nodes[a], self.nodes[b])
            current_color = None

    def position(self, x_id, k):
        '''
        query k step position of x
        x_id: id of query node
        k: [0, +\infinity)
        return: the node ID the kth-step position
        '''
        x = self.nodes[x_id]
        x_depth = depth(x)
        if k < 0:
            print("k can not be negative")
            return None
        elif k >= 0 and k <= x_depth:
            # result are in the root path
            return kth_ancestor(x, k)._id
        else:
            # result in the cycle
            k = k - x_depth - 1  # for query s
            root = self.nodes[find_root(x)._id]
            if root in self.root_ptr:
                # the lowest decenstor in the cycle
                s_id = self.root_ptr[root._id]
                s = self.nodes[s_id]
                s_depth = depth(s)
                cycle_length = s_depth + 1
                k = k % cycle_length
                return kth_ancestor(s, k)._id
            else:
                return -1  # a dead node

    def re_link(self, x_id, y_id):
        '''
        update a neighbor of node x_id
        udpate x's pointer to y
        x_id: id of x
        y_id: id of y
        totally 5 cases
        '''
        x = self.nodes[x_id]
        y = self.nodes[y_id]
        root_x = find_root(x)
        root_y = find_root(y)
        if self.is_in_cycle(x_id):
            if not is_root(x):
                # make x the root of current tree
                cut(x)
                s = self.nodes[self.root_ptr[root_x._id]]
                link(root_x, s)
                self.root_ptr.pop(root_x._id)
                self.root_ptr[x_id] = None  # tmp
            if root_x == root_y:
                # same tree
                self.root_ptr[x_id] = y_id
            else:
                # different tree, so merge them
                link(x, y)
                self.root_ptr.pop(x_id)  # remove key

        else:
            # x is not in the cycle
            # so x must not be a root
            cut(x)
            link(x, y)

    def is_in_cycle(self, x_id):
        '''
        whether x is in the cycle of its WCC
        '''
        s = self.get_children(x_id)  # the children
        x = self.nodes[x_id]
        return lca(s, x) == x

    def get_children(self, x_id):
        '''
        get the particular children of x_id's WCC
        which is lowest children in the cycle
        '''
        x = self.nodes[x_id]
        r = find_root(x)
        s = self.nodes[self.root_ptr[r._id]]
        return s

    def show(self):
        '''
        show nodes of current index
        '''
        print("#WCC", len(self.root_ptr))
        print("N", len(self.nodes))


# test code
def make_tree(n):
    nodes = []
    for i in range(0, n):
        node = Node(label=i)
        nodes.append(node)
        if i > 0:
            link(nodes[i], nodes[i - 1])
    return nodes


def clique(n):
    '''
    create a directed graph of clique of size n
    '''
    g = nx.complete_graph(n, create_using=nx.DiGraph())
    return g


edge_list = [(4, 0), (0, 1), (1, 4), (1, 3), (0, 2), \
             (5, 6), (5, 7), (6, 5)]


def test():
    print("link cut tree test")
    # g = nx.from_edgelist(edge_list, \
    #         create_using=nx.DiGraph())
    g = load_csr_g("ca-GrQc")
    dfg = DFG_Index(g)
    dfg.show()
    query = (7, 8)
    r = dfg.position(*query)
    print(query, dfg.position(*query))
    print(type(r))

    # nodes = []
    # for i in range(0,100):
    #     node = Node(label=i)
    #     # print("node", i)
    #     nodes.append(node)
    #     update(nodes[i])
    #     if i > 0:
    #         link(nodes[i], nodes[i-1])
    # # print(kth_ancestor(nodes[2], 2).label)
    # print(lca(nodes[90], nodes[3]).label)
    # cut(nodes[50])
    # a = lca(nodes[90], nodes[3])
    # if a is not None:
    #     print(a.label)
    # link(nodes[50], nodes[4])
    # print(lca(nodes[90], nodes[8]).label)
    return


if __name__ == '__main__':
    test()
