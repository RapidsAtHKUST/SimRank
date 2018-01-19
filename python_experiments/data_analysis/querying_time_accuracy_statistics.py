import json
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

bflpmc_tag = 'bflpmc'
flpmc_tag = 'flpmc'
bprw_tag = 'bprw'
sling_tag = 'sling'
lind_tag = 'lind'
cw_tag = 'cw'
isp_tag = 'isp'
tsf_tag = 'tsf'

pair_num_lst = [10 ** 4, 10 ** 5, 10 ** 6]


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
    algorithm_lst = [bflpmc_tag, flpmc_tag, bprw_tag]
    stat_folder = other_algo_indexing_stat_root_folder
    for algorithm in algorithm_lst:
        if result_file_name.startswith(algorithm):
            stat_folder = our_algo_indexing_stat_root_folder

    for round_i in xrange(round_num):
        my_path = os.sep.join(map(str, [stat_folder, data_set, pair_num, round_i, result_file_name]))
        # use the first one for this round
        accuracy_lst.append(get_tag_info(my_path, 'max err', lambda x: x[0]))
    return accuracy_lst


def get_accuracy_lst_per_algorithm(round_num, result_file_name):
    accuracy_dict_lst = []
    for data_set in accuracy_data_set_lst:
        accuracy_dict_lst.append(
            dict(zip(pair_num_lst, map(lambda pair_num: get_accuracy_lst(data_set, pair_num, round_num,
                                                                         result_file_name), pair_num_lst))))
    return dict(zip(data_set_lst, accuracy_dict_lst))


def write_accuracy_dict_to_json():
    algorithm_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag, isp_tag, tsf_tag, lind_tag, cw_tag]

    def get_ground_truth_suffix(algorithm):
        if algorithm is lind_tag:
            return '-rand-ben-gt.txt'
        elif algorithm is cw_tag:
            return '-rand-gen-gt.txt'
        else:
            return '-rand-bench-gt.txt'

    os.system('mkdir -p data-json')
    dict_lst = map(lambda algorithm: get_accuracy_lst_per_algorithm(10, algorithm + get_ground_truth_suffix(algorithm)),
                   algorithm_lst)

    accuracy_algorithm_dict = json.dumps(dict(zip(algorithm_lst, dict_lst)), indent=4)
    if len(pair_num_lst) > 1:
        with open('data-json/accuracy_result_full.json', 'w') as ofs:
            ofs.write(accuracy_algorithm_dict)
    else:
        with open('data-json/accuracy_result.json', 'w') as ofs:
            ofs.write(accuracy_algorithm_dict)


if __name__ == '__main__':
    write_accuracy_dict_to_json()
    pair_num_lst = [10 ** 5]
    write_accuracy_dict_to_json()
