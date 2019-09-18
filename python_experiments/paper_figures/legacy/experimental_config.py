datasets = [
    "ca-GrQc",
    "ca-HepTh",
    "p2p-Gnutella06",
    "wiki-Vote",
    "email-Enron",
    "email-EuAll",
    "web-NotreDame",
    "web-Stanford",
    "web-BerkStan",
    "web-Google",
    "cit-Patents",
    "soc-LiveJournal1",
]
size_g = {
    "ca-GrQc": (5242, 14496),
    "p2p-Gnutella06": (8717, 31525),
    "ca-HepTh": (9877, 25998),
    "wiki-Vote": (7115, 103689),
    "web-NotreDame": (325729, 1497134),
    "web-Stanford": (281903, 2312497),
    "web-BerkStan": (685230, 7600595),
    "web-Google": (875713, 5105039),
    "cit-Patents": (3774768, 16518948),
    "soc-LiveJournal1": (4847571, 68993773),
    "email-Enron": (36692, 183831),
    "email-EuAll": (265214, 420045)
}
data_names = {
    "ca-GrQc": "CG",
    "p2p-Gnutella06": "PG",
    "ca-HepTh": "CH",
    "wiki-Vote": "WV",
    "web-NotreDame": "WN",
    "web-Stanford": "WS",
    "web-BerkStan": "WB",
    "web-Google": "WG",
    "cit-Patents": "CP",
    "soc-pokec-relationships": "PR",
    "soc-LiveJournal1": "LJ",
    "email-Enron": "EN",
    "email-EuAll": "EU",
}
data_type = {
    "ca-GrQc": "u",
    "ca-HepTh": "u",
    "p2p-Gnutella06": "d",
    "wiki-Vote": "d",
    "email-Enron": "u",
    "email-EuAll": "d",
    "web-NotreDame": "d",
    "web-Stanford": "d",
    "web-BerkStan": "d",
    "web-Google": "d",
    "cit-Patents": "d",
    "soc-LiveJournal1": "d",
}

root_dir = '/home/yche/mnt/wangyue-clu/export/data/ywangby/legacy-simrank-results/datasets'
LOCAL_PUSH_DIR = "{}/local_push/".format(root_dir)
EFFECTIVE_DIR = "{}/effective_error/".format(root_dir)
TKDE17_DIR = "{}/tkde17/".format(root_dir)
DYNAMIC_LOCAL_PUSH_DIR = "{}/dynamic_exp/".format(root_dir)
ICDE14_DIR = "{}/icde_dynamic/".format(root_dir)
LINEARD_DIR = "{}/linearD/".format(root_dir)
CLOUD_WALKER_DIR = "{}/cloudwalker/".format(root_dir)

LEGEND_SIZE = 22
LABEL_SIZE = 22
TICK_SIZE = 22
FIG_SIZE_SINGLE = (8, 4)
FIG_SIZE_MULTIPLE = (32, 4)
MARKER_SIZE = 18
