from indexing_time_space_statistics import get_tag_info, data_set_lst, index_size_tag, index_time_tag, v_num_dict, \
    format_str
from reads_accuracy_statistics import *
import os

reads_efficiency_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/' \
                          'yche/git-repos/SimRank/python_experiments/exp_results/reads_01_24_efficiency'

index_root_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/LinsysSimRank/datasets'

index_folder_dict = {
    reads_tag: 'reads',
    reads_d_tag: 'readsd',
    reads_rq_tag: 'readsrq'
}


## 1st: indexing time
def get_index_time(data_set, algorithm_name):
    algorithm_log_name = algorithm_name + suffix_str
    file_path = os.sep.join([reads_efficiency_folder, data_set, '10000', '0', algorithm_log_name])
    indexing_time = get_tag_info(file_path, 'indexing time')
    if indexing_time is None:
        return 999999999999999.
    return indexing_time


def get_index_time_dict(algorithm_name):
    return dict(zip(data_set_lst, map(lambda data_set: get_index_time(data_set, algorithm_name), data_set_lst)))


## 2nd: index size
def get_index_size(data_set, algorithm_name):
    index_folder_path = os.sep.join([index_root_folder, index_folder_dict[algorithm_name]])
    if algorithm_name in [reads_tag, reads_d_tag]:
        index_naming = index_folder_path + os.sep + '_'.join(
            map(str, [data_set, v_num_dict[data_set], 1236, '0.600000', 10])) + '.bin'
    else:
        index_naming = index_folder_path + os.sep + '_'.join(
            map(str, [data_set, v_num_dict[data_set], 100, '0.600000', 10])) + '.bin'
    return float(format_str(os.path.getsize(index_naming) / (1024. ** 2)))


# collected information (in case that we need to remove large files)
reads_d_size_dict = {
    'ca-GrQc': 298646704,
    'ca-HepTh': 532953712,
    'wiki-Vote': 142430960,
    'p2p-Gnutella06': 299125744,
    'email-Enron': 1364548016,
    'email-EuAll': 4374869768,
    'web-BerkStan': 27234698312,
    'web-Google': 29823990656,
    'web-NotreDame': 8626791720,
    'web-Stanford': 12135352960,
    'cit-Patents': 104277969456,
    'soc-LiveJournal1': 197627796 * 1024
}


def get_index_size_dict(algorithm_name):
    if algorithm_name in [reads_d_tag]:
        return dict(zip(data_set_lst, map(lambda data_set: reads_d_size_dict[data_set] / (1024. ** 2), data_set_lst)))
    return dict(zip(data_set_lst, map(lambda data_set: get_index_size(data_set, algorithm_name), data_set_lst)))


def get_algorithm_index_info_dict(algorithm_name):
    return {
        index_time_tag: get_index_time_dict(algorithm_name),
        index_size_tag: get_index_size_dict(algorithm_name)
    }


if __name__ == '__main__':
    data_set = data_set_lst[-1]
    algorithm_tag_lst = [reads_tag, reads_d_tag, reads_rq_tag]

    with open('data-json/index_result_reads.json', 'w') as ofs:
        ofs.write(json.dumps(
            dict(zip(algorithm_tag_lst, map(lambda tag: get_algorithm_index_info_dict(tag), algorithm_tag_lst))),
            indent=4))
