import os

from data_analysis.generate_accuracy_markdown import get_tag_info, reads_d_tag, reads_rq_tag, json, \
    bprw_tag, bflpmc_tag, flpmc_tag, cw_tag, isp_tag, lind_tag, sling_tag, tsf_tag
from data_analysis.probesim_querying_time_statistics import probesim_tag

pair_num_lst = [10 ** 3, 10 ** 4, 10 ** 5, 10 ** 6]

query_time_tag = 'total query cpu time'
data_set_lst = ['digg-friends', 'flickr-growth']
suffix_str = '-rand-bench.txt'

# reads related
reads_efficiency_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/new-git-repos-yche/SimRank/' \
                          'python_experiments/exp_results/reads_08_03_efficiency'
our_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/new-git-repos-yche/SimRank/' \
             'python_experiments/exp_results/our_methods_overview_08_02'
other_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/new-git-repos-yche/SimRank/' \
               'python_experiments/exp_results/other_methods_overview_07_31'

reads_lst = [reads_d_tag, reads_rq_tag]
our_algorithm_lst = [bprw_tag, bflpmc_tag, flpmc_tag]
other_algorithm_lst = [cw_tag, isp_tag, lind_tag, sling_tag, tsf_tag]
probesim_lst = [probesim_tag]


def get_time_lst(data_set, pair_num, round_num, result_file_name, algorithm_name):
    cpu_time_lst = []
    stat_folder = reads_efficiency_folder if algorithm_name in reads_lst else (
        our_folder if algorithm_name in our_algorithm_lst else other_folder)
    for round_i in xrange(round_num):
        my_path = os.sep.join(map(str, [stat_folder, data_set, pair_num, round_i, result_file_name]))
        # use the first one for this round
        querying_time = get_tag_info(my_path, query_time_tag, min)
        if querying_time is None:
            querying_time = 999999999999999
        cpu_time_lst.append(querying_time)
    return cpu_time_lst


def get_suffix(algorithm_name):
    if algorithm_name in [lind_tag]:
        return '-rand-ben.txt'
    if algorithm_name in [cw_tag]:
        return '-rand-gen.txt'
    if algorithm_name in [probesim_tag]:
        return '.txt'
    return suffix_str


def get_time_lst_per_algorithm(round_num, result_file_name):
    query_time_dict_lst = []
    for data_set in data_set_lst:
        query_time_dict_lst.append(
            dict(zip(pair_num_lst,
                     map(lambda pair_num:
                         get_time_lst(data_set, pair_num, round_num,
                                      result_file_name + get_suffix(result_file_name), result_file_name),
                         pair_num_lst))))
    return dict(zip(data_set_lst, query_time_dict_lst))


if __name__ == '__main__':
    querying_dict = {
        reads_d_tag: get_time_lst_per_algorithm(1, reads_d_tag),
        reads_rq_tag: get_time_lst_per_algorithm(1, reads_rq_tag)
    }
    for algorithm in our_algorithm_lst + other_algorithm_lst + probesim_lst:
        querying_dict[algorithm] = get_time_lst_per_algorithm(1, algorithm)
    os.system('mkdir -p ../data-json/new_datasets/')
    with open('../data-json/new_datasets/query_result_full_cpu_time_all.json', 'w') as ofs:
        ofs.write(json.dumps(querying_dict, indent=4))
