# script that do some stuff
from data import compute_ground_truth_SimRank, DATASETS


def compute_sim():
    for d in DATASETS:
        print("computing true simrank: ", d)
        compute_ground_truth_SimRank(d[0])
        print("finish...")


if __name__ == '__main__':
    compute_sim()
