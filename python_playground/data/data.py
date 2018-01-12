# load data from datasets
import os
import pickle
from pprint import pprint

from linearization import *

C = 0.6
BASE_DIR = "./datasets/"
UPPER_BOUND_OF_EDGES = 222000
DATASETS = [
    ("ca-AstroPh", "u"),
    ("ca-HepPh", "u"),
    ("facebook_combined", "u"),
    ("com-dblp-ungraph", "u"),
    ("test_g", "d"),
    ("ca-GrQc", "u"),
    ("odlis", "d"),
    ("as20000102", "d"),
    ("p2p-Gnutella06", "d"),
    ("web-NotreDame", "d"),
    ("web-Stanford", "d"),
    ("web-BerkStan", "d"),
    ("ca-HepTh", "u"),
    ("web-Google", "d"),
    ("wiki-Vote", "d"),
    ("soc-pokec-relationships", "d"),
    ("soc-LiveJournal1", "d"),
    ("soc-Slashdot0811", "d"),
]

DATA = ["odlis", "ca-GrQc", "p2p-Gnutella06", "ca-HepTh", "wiki-Vote", "web-NotreDame", "web-Stanford", \
        "web-BerkStan", "web-Google", "soc-pokec-relationships", "soc-LiveJournal1", \
        "ca-AstroPh", "ca-HepPh", "facebook_combined", "com-dblp.ungraph"]
ALL_PATH = [BASE_DIR + d[0] + ".txt" for d in DATASETS]


def get_adj_file_path(data_name, is_tranpose=True):
    '''
    is_tranpose: wether load A.T
    '''
    if is_tranpose:
        return BASE_DIR + 'adj_T/' + data_name + '.npz'
    else:
        return BASE_DIR + 'adj/' + data_name + '.npz'


def get_edge_list_path(data_name):
    return BASE_DIR + "edge_list/" + data_name + ".txt"


def get_adj_F_file_path(data_name, is_shurnken=False):
    '''
    is_shurnken: whether use the reduced linear system
    '''
    if is_shurnken:
        return BASE_DIR + "adj_Fs/" + data_name + ".npz"
    else:
        return BASE_DIR + "adj_F/" + data_name + ".npz"


def get_D_path(data_name):
    # return the diagonal correction matrix
    return BASE_DIR + "D/" + data_name + ".npy"


def load_ground_truth(data_name):
    '''
    load the ground truth matrix given data_name
    '''
    normal_file_name = BASE_DIR + "ground_truth_SimRank/" + data_name + '.npy'
    symmetric_file_name = BASE_DIR + "ground_truth_SimRank/" + data_name + "_sym" + '.npy'
    if os.path.exists(normal_file_name):
        print("loading noraml ground truth")
        return np.load(normal_file_name)
    elif os.path.exists(symmetric_file_name):
        print("loading symmetric ground truth")
        return np.load(symmetric_file_name)
    else:
        print("no ground truth for data_name")
        return


def compute_ground_truth_SimRank(data_name, is_shurnken=False, adj_F_path=None):
    '''
    data_name: name of the datsets
    use_sym: whether use the reduced linear system
    adj_F_path: the path of the adj F matrix
    '''
    adj_file_path = get_adj_file_path(data_name)
    print("computing groud truth for " + adj_file_path)
    if is_shurnken:
        file_name = BASE_DIR + "ground_truth_SimRank/" + data_name + "_sym" + '.npy'
    else:
        file_name = BASE_DIR + "ground_truth_SimRank/" + data_name + '.npy'
    if not os.path.exists(adj_file_path):
        print("The adj file does not exists...")

    if os.path.exists(file_name):
        print("already computed, ignore")
        return
    # load the adj file
    if adj_F_path is None:
        if not os.path.exists(get_adj_F_file_path(data_name, is_shurnken)):
            print("kroneck product has not been computed... exit now")
            return
        else:
            print("loading adj F matrix")
            adj_F = load_sparse_csr(get_adj_F_file_path(data_name, is_shurnken))
    else:
        print("loading adj F matrix !!!!!", adj_F_path)
        adj_F = load_sparse_csr(adj_F_path)
    print("finish...")
    print("loading adj matrix")
    adj_mat = load_sparse_csr(adj_file_path)
    print("finish...")

    # compute ground truth
    if adj_mat.nnz < UPPER_BOUND_OF_EDGES:
        print("Shape of the matrix:  ", adj_mat.shape)
        if is_shurnken:
            S = symmetric_simrank(adj_mat, C, adj_F)
        else:
            S = simrank(adj_mat, C, F=adj_F)  #
        # save to file system
        print("saving to file " + file_name)
        np.save(file_name, S)
        # save_sparse_csr(file_name, S)
        pprint(S)
    else:
        print("graph too large to comput true simrank scores")


def compute_adj_F(data_name, is_shurnken=False, file_path=None):
    '''
    is_shurnken: whether use shunken linear system
    '''
    print("computing Adj F for data: ", data_name)
    adj_file_path = get_adj_file_path(data_name)
    if file_path is None:
        file_name = get_adj_F_file_path(data_name, is_shurnken)
    else:
        file_name = file_path + data_name
    if os.path.exists(file_name):
        print("already computed, ignore")
        return
    if os.path.exists(adj_file_path):
        adj_mat = load_sparse_csr(adj_file_path)  # load A.T
        if adj_mat.nnz < UPPER_BOUND_OF_EDGES:
            print("Shape of the matrix:  ", adj_mat.shape)
            if not is_shurnken:
                AF = csr_kron_F(adj_mat, adj_mat)
            else:
                # AF = symmetric_csr_kron(adj_mat)
                # use the parallel method
                AF = parallel_symmetric_csr_kron(adj_mat)
            # save to file system
            print("adj_F: ", matrix_info(AF))
            print("saving to file " + file_name)
            save_sparse_csr(file_name, AF)
            pprint(AF)
        else:
            print("graph too large to comput true simrank scores")
    else:
        print("The adj file does not exists...")


def make_path(name):
    '''
    name: datasets name
    '''
    path = './datasets/' + name + '.txt'
    return path


def get_name(path):
    '''
    :param path: the path of the data file
    :return: (dirname, dataname)
    '''
    dirname, basename = os.path.split(path)
    data_name = basename.split('.')[0]
    print(dirname, basename, data_name)
    return (dirname, data_name)


def load_and_convert_from_edge_file(path, directed=True, comments='#'):
    print("loading ", path)
    if directed == True:
        print("It's a directed graph...")
        g = nx.read_edgelist(path, create_using=nx.DiGraph(), comments=comments)
    else:
        g = nx.read_edgelist(path)
    return nx.convert_node_labels_to_integers(g)  # re-order the nodes


def convert_and_save_to_adj_matrix(path, transpose=True):
    '''
    :param path: given SNAP dataset
    transpose: whether save as transpose of A, i.e. A.T
    :return: save it to Adjformat
    '''
    print("converting to adj matrix...", path)
    g = load_and_convert_from_edge_file(path, comments='%')
    dirname, data_name = get_name(path)
    if transpose:
        new_path = dirname + "/adj_T/" + data_name
    else:
        new_path = dirname + "/adj/" + data_name
    print("converting to adj matrix...")
    adj_m = nx.to_scipy_sparse_matrix(g, dtype="bool", format="csc")
    if transpose:
        print("computing A.T")  # since we need inneighbors matrix, save A.T as csr
        adj_m = adj_m.transpose().tocsr()
    else:
        adj_m = adj_m.tocsr()
    print("adj_m", matrix_info(adj_m))
    # nx.write_adjlist(g, new_path)
    save_sparse_csr(new_path, adj_m)
    print("finish")
    return


def compute_Diagonal_correction_array(data_name, c=C):
    '''
    compute D for given dataset
    '''
    adj_file_path = BASE_DIR + "adj/" + data_name + '.npz'
    D_file_path = get_D_path(data_name)
    # if os.path.exists(D_file_path):
    #     print("D is already computed..")
    #     return
    if os.path.exists(adj_file_path):
        adj_mat = load_sparse_csr(adj_file_path)
        print("load adj csr file success", adj_file_path)
        g = nx.from_scipy_sparse_matrix(adj_mat, create_using=nx.DiGraph())
        print("computing D")
        D = compute_D(g)
        print("finish.. saving to " + D_file_path)
        np.save(D_file_path, D)
    else:
        print("data file does not exist")


def load_graph(data_name):
    '''
    load graph with networkx format
    '''
    adj_path = get_adj_file_path(data_name,
                                 is_tranpose=False)
    csr_matrx = load_sparse_csr(adj_path)
    print("constructing graph..", data_name)
    g = nx.from_scipy_sparse_matrix(csr_matrx, \
                                    create_using=nx.DiGraph())
    print("constructing complete")
    return g


def load_csr_g(data_name, is_tranpose=False):
    adj_path = get_adj_file_path(data_name,
                                 is_tranpose=is_tranpose)
    csr_matrx = load_sparse_csr(adj_path)
    return csr_matrx


def write_to_edgelist(data_name):
    print("converting " + data_name + "to edge list")
    g = load_graph(data_name)
    nx.write_edgelist(g, get_edge_list_path(data_name), data=False)
    print("convertion complete.")
    return


def test():
    pass


def data_info():
    '''
    load the data set and save the statistical information
    '''
    info = dict()
    for data in DATA:
        print("loading...", data)
        a = load_sparse_csr(get_adj_file_path(data, is_tranpose=False))
        info[data] = (a.shape[0], a.nnz)
    pickle.dump(info, open("./data_info.pickle", "wb"))
    return


if __name__ == '__main__':
    # D_times = []
    # convert_and_save_to_adj_matrix("./datasets/p2p-Gnutella06.txt", False)
    # from experiments import DATA
    # for data in DATA:
    #     t1 = time.time()
    #     compute_Diagonal_correction_array(data)
    #     t2 = time.time()
    #     D_times.append(t2-t1)
    # print(D_times)
    # compute_Diagonal_correction_array("odlis")
    for p in ["wiki-Link"]:
        convert_and_save_to_adj_matrix("./datasets/" + p + ".txt")
        convert_and_save_to_adj_matrix("./datasets/" + p + ".txt", False)
        write_to_edgelist(p)
    # with Pool(len(DATA)) as pool:
    #    pool.map(write_to_edgelist, DATA)
    # compute_ground_truth_SimRank("odlis")
    # compute_ground_truth_SimRank("odlis", is_shurnken=True)
    # convert_and_save_to_adj_matrix(make_path("ca-HepTh"))
    # DATA = ["odlis", "ca-GrQc", "p2p-Gnutella06", "ca-HepTh",\
    #         "wiki-Vote"]
    # for data in DATA[0:5]:
    #     compute_adj_F(data, is_shurnken=True)
    #     compute_ground_truth_SimRank(data)
    # data = "ca-GrQc"
    # load_ground_truth(data)
    # compute_adj_F("wiki-Vote", is_shurnken=True, file_path="/home/Keith/wiki-Vote-sym.npz")
    # compute_adj_F("wiki-Vote", "/home/Keith/wiki-Vote.npz")
    # compute_ground_truth_SimRank("ca-GrQc", is_shurnken=True)
    # compute_ground_truth_SimRank("ca-HepTh")
    # compute_ground_truth_SimRank("wiki-Vote", is_shurnken=True, adj_F_path="/home/buaawangyue/wiki-Vote.npzwiki-Vote.npz")
    # compute_ground_truth_SimRank("wiki-Vote", "/home/Keith/wiki-Vote.npz")
    # data_info()
