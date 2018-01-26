import os

if __name__ == '__main__':
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
