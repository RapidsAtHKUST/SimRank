from querying_time_accuracy_statistics import get_accuracy_lst, accuracy_data_set_lst, pair_num_lst
import json

reads_tag = 'reads-s'
reads_d_tag = 'reads-d'
reads_rq_tag = 'reads-rq'
suffix_str = '-rand-bench.txt'
accuracy_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/git-repos/SimRank/python_experiments/exp_results/reads_01_24'


def get_accuracy_lst_per_algorithm(round_num, result_file_name):
    pair_num_lst = [100000]

    accuracy_dict_lst = []
    for data_set in accuracy_data_set_lst:
        accuracy_dict_lst.append(
            dict(zip(pair_num_lst, map(lambda pair_num:
                                       get_accuracy_lst(data_set, pair_num, round_num, result_file_name,
                                                        stat_folder=accuracy_folder), pair_num_lst))))
    return dict(zip(accuracy_data_set_lst, accuracy_dict_lst))


if __name__ == '__main__':
    algorithm_tag_lst = [reads_tag, reads_d_tag, reads_rq_tag]
    accuracy_info_lst = [get_accuracy_lst_per_algorithm(10, algorithm + suffix_str) for algorithm in algorithm_tag_lst]
    with open('data-json/accuracy_result_reads.json', 'w') as ofs:
        ofs.write(json.dumps(dict(zip(algorithm_tag_lst, accuracy_info_lst)), indent=4))
