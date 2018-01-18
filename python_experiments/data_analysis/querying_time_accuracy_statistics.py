import os

data_set_lst = [
    'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
    'email-Enron', 'email-EuAll',
    'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google',
    'cit-Patents', 'soc-LiveJournal1']

accuracy_data_set_lst = data_set_lst[0:4]

our_algo_indexing_stat_root_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/' \
                                     'yche/git-repos/SimRank/python_experiments/exp_results/' \
                                     'our_methods_overview_01_17'

other_algo_indexing_stat_root_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/' \
                                       'yche/git-repos/SimRank/python_experiments/exp_results/' \
                                       'other_methods_overview_01_17'


def get_tag_info(file_path, tag, functor):
    if not os.path.exists(file_path):
        return None
    with open(file_path) as ifs:
        # assume unit seconds
        lst = map(lambda line: eval(line.split(':')[-1].split('s')[0]),
                  filter(lambda line: line.startswith(tag), ifs.readlines()))
        return None if len(lst) == 0 else functor(lst)


def get_accuracy_lst(data_set, pair_num, round_num, result_file_name):
    accuracy_lst = []
    for round_i in xrange(round_num):
        my_path = os.sep.join(map(str, [our_algo_indexing_stat_root_folder, data_set, pair_num, round_i,
                                        result_file_name]))
        # use the first one for this round
        accuracy_lst.append(get_tag_info(my_path, 'max err', lambda x: x[0]))
    return accuracy_lst


def get_accuracy_lst_per_algorithm(round_num, result_file_name):
    accuracy_dict_lst = []
    for data_set in accuracy_data_set_lst:
        pair_num_lst = [10 ** 4, 10 ** 5, 10 ** 6]
        print data_set
        accuracy_dict_lst.append(
            dict(zip(pair_num_lst, map(lambda pair_num: get_accuracy_lst(data_set, pair_num, round_num,
                                                                         result_file_name), pair_num_lst))))
    return dict(zip(data_set_lst, accuracy_dict_lst))


if __name__ == '__main__':
    pair_num_lst = [10 ** 4, 10 ** 5, 10 ** 6]
    our_algorithm_lst = ['bflpmc', 'flpmc', 'bprw']
    ground_truth_suffix_str = '-rand-bench-gt.txt'

    os.system('mkdir -p data-json')
    print get_accuracy_lst_per_algorithm(10, 'bprw-rand-bench-gt.txt')
