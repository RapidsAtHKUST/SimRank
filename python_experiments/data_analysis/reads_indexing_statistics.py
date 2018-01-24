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
        index_naming = ''
    return float(format_str(os.path.getsize(index_naming) / (1024. ** 2)))


def get_index_size_dict(algorithm_name):
    return dict(zip(data_set_lst, map(lambda data_set: get_index_size(data_set, algorithm_name), data_set_lst)))


if __name__ == '__main__':
    data_set = data_set_lst[-1]
    print get_index_time_dict(reads_tag)
    print get_index_size_dict(reads_tag)
