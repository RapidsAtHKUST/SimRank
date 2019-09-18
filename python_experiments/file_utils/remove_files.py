import os


def remove_files0():
    root_dir = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/git-repos/SimRank/python_experiments/exp_results/varying_eps_exp'
    data_set = 'ca-GrQc'
    pair_num = 100000
    eps_lst = list(([0.001 * (i + 1) for i in xrange(30)]))
    file_name_lst = ['reads-d-rand-bench.txt', 'reads-rq-rand-bench.txt']

    for eps in eps_lst:
        read_d_file = os.sep.join(map(str, [root_dir, data_set, pair_num, eps, file_name_lst[0]]))
        read_rq_file = os.sep.join(map(str, [root_dir, data_set, pair_num, eps, file_name_lst[1]]))
        os.system('rm ' + read_d_file)
        os.system('rm ' + read_rq_file)


if __name__ == '__main__':
    tag = 'exp_results'
    folder_name = 'varying_eps_for_topk_precision_exp'
    sample_num = str(10 ** 4)
    k = str(800)
    data_set_name_lst = [
        'ca-GrQc',
        'ca-HepTh',
        'p2p-Gnutella06',
        'wiki-Vote'
    ]
    eps_lst = list(reversed([0.0001, 0.0004, 0.0016, 0.0064, 0.0256]))

    for algorithm in [
        # 'reads-rq-rand-bench', 'reads-d-rand-bench',
        'reads-d-rand-bench-gt']:
        for data_set_name in data_set_name_lst:
            for eps in eps_lst:
                statistics_dir = os.sep.join(map(str, [
                    '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/git-repos/SimRank/python_experiments/',
                    tag, folder_name, sample_num, k, data_set_name, eps]))
                statistics_file_path = statistics_dir + os.sep + algorithm + '.txt'
                os.system('rm ' + statistics_file_path)
