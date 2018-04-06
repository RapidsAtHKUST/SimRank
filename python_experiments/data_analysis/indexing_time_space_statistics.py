import decimal
import json

import os

data_set_lst = [
    'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
    'email-Enron', 'email-EuAll',
    'web-NotreDame',
    'web-Stanford', 'web-BerkStan', 'web-Google',
    'cit-Patents', 'soc-LiveJournal1',
    'wiki-Link']

# for tsf indexing space computation
v_num_dict = dict(zip(data_set_lst, [5242, 9877, 8717, 7115,
                                     36692, 265214,
                                     325729,
                                     281903, 685230, 875713,
                                     3774768, 4847571,
                                     12150976]))
size_of_int = 4

our_algo_indexing_stat_root_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/' \
                                     'yche/git-repos/SimRank/python_experiments/plp_parallel_gen_idx_0405'

other_algo_indexing_stat_root_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/' \
                                       'yche/git-repos/SimRank/python_experiments/exp_results/' \
                                       'other_methods_overview_01_16'
other_algo_indexing_stat_root_folder_new = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/' \
                                           'yche/git-repos/SimRank/python_experiments/exp_results/' \
                                           'other_methods_overview_01_17'
rand_pair_num_str = "1000"
rand_round = "0"

datasets_root_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/LinsysSimRank/datasets'
datasets_root_folder_backup = '/home/yche/mnt/wangyue-clu/export/data/datasets'

index_time_tag = "indexing time"
index_size_tag = "index disk size"
max_mem_size_tag = "max memory consumption"
local_push_tag = "full local push"
sling_tag = "sling"
linear_d_tag = "linear d"
cloud_walker_tag = "cloud walker"
tsf_tag = "tsf"


def format_str(float_num):
    return str(decimal.Decimal.from_float(float_num).quantize(decimal.Decimal('0.000')))


def ms_val_to_s(num):
    return eval(str(decimal.Decimal.from_float(float(num) / 1000).quantize(decimal.Decimal('0.0000'))))


def get_tag_info(file_path, tag):
    # print file_path
    if not os.path.exists(file_path):
        return None
    with open(file_path) as ifs:
        # assume unit seconds
        lst = map(lambda line: eval(line.split(':')[-1].split('s')[0]),
                  filter(lambda line: line.startswith(tag), ifs.readlines()))
        return None if len(lst) == 0 else min(lst)


def get_tag_no_colon_info(file_path, tag):
    if not os.path.exists(file_path):
        return None
    with open(file_path) as ifs:
        # assume unit seconds
        lst = map(lambda line: eval(line.split(tag)[-1].split('s')[0]),
                  filter(lambda line: line.startswith(tag), ifs.readlines()))
        return None if len(lst) == 0 else min(lst)


# introduction: isp and our bprw are on-line algorithms

# 1st: our full local push related (for bflpmc, flpmc)
class LocalPushIndexingStat:
    # unit: seconds
    @staticmethod
    def get_indexing_time():
        indexing_time_lst = []
        for data_set in data_set_lst:
            # algorithm_name = 'pflp'
            algorithm_name = 'prlp'
            indexing_time = get_tag_info(
                os.sep.join([our_algo_indexing_stat_root_folder, algorithm_name, data_set, '1.txt']),
                tag='computation time')
            indexing_time_lst.append(indexing_time)
        return dict(zip(data_set_lst, indexing_time_lst))

    # unit: MB
    @staticmethod
    def get_index_disk_size():
        local_push_folder = os.sep.join([datasets_root_folder_backup, 'local_push'])
        space_size_lst = []
        for data_set in data_set_lst:
            if data_set == 'wiki-Vote':
                space_size_lst.append(float(format_str(2 * (174772 + 7735008) / (1024 ** 2))))
            elif data_set == 'wiki-Link':
                space_size_lst.append(float(format_str(2 * (2237183088 + 50408583172) / (1024 ** 2))))
            else:
                index_naming = '_'.join(['FLP', '-'.join([data_set, '0.600', '0.116040'])])
                p_size = os.path.getsize(os.sep.join([local_push_folder, index_naming + '.P'])) / (1024. ** 2)
                r_size = os.path.getsize(os.sep.join([local_push_folder, index_naming + '.R'])) / (1024. ** 2)
                space_size_lst.append(float(format_str(p_size + r_size)))
        return dict(zip(data_set_lst, space_size_lst))


# 2nd: sling related
class SlingIndexingStat:
    # unit: seconds
    @staticmethod
    def get_indexing_time():
        # sling's parallel d calculation not effective, thus we collect single thread time also
        sling_compute_d_time_lst = [24.868744, 39.719319, 3.928212, 1.050985,
                                    100.487657, 8.268188,
                                    820.528155, 1578.569044, 3371.547423, 2450.991423,
                                    1693.183911, 13469.924516, 14415.177543]
        sling_compute_d_time_dict = dict(zip(data_set_lst, sling_compute_d_time_lst))
        indexing_time_lst = []
        for data_set in data_set_lst:
            for algorithm_name in ['sling_all', 'sling_bench']:
                parallel_cal_d_time = get_tag_no_colon_info(
                    os.sep.join([other_algo_indexing_stat_root_folder, data_set, algorithm_name + '.txt']),
                    tag='finish calcD')
                backward_time = get_tag_no_colon_info(
                    os.sep.join([other_algo_indexing_stat_root_folder, data_set, algorithm_name + '.txt']),
                    tag='finish backward')
                if backward_time is not None:
                    if sling_compute_d_time_dict[data_set] == 0:
                        indexing_time_lst.append(float(format_str(6 * parallel_cal_d_time + backward_time)))
                    else:
                        indexing_time_lst.append(float(format_str(sling_compute_d_time_dict[data_set] + backward_time)))
                    break
        return dict(zip(data_set_lst, indexing_time_lst))

    # unit: MB
    @staticmethod
    def get_index_disk_size():
        index_folder = os.sep.join([datasets_root_folder_backup, 'sling'])
        space_size_lst = []
        # use the previous values, since we remove *.p for saving space
        index_dict = {
            "wiki-Vote": 6.362,
            "cit-Patents": 5559.157,
            "email-EuAll": 211.237,
            "web-Stanford": 2999.271,
            "ca-HepTh": 127.29,
            "web-BerkStan": 7392.115,
            "web-NotreDame": 4187.683,
            "p2p-Gnutella06": 115.335,
            "soc-LiveJournal1": 44850.682,
            "web-Google": 6957.387,
            "ca-GrQc": 62.395,
            "email-Enron": 409.886
        }
        for data_set in data_set_lst:
            if data_set in index_dict:
                space_size_lst.append(index_dict[data_set])
            else:
                index_naming = '_'.join(['RLP', '-'.join([data_set, '0.600', '0.002000', '0.000290'])])
                d_size = os.path.getsize(os.sep.join([index_folder, index_naming + '.d'])) / (1024. ** 2)
                p_size = os.path.getsize(os.sep.join([index_folder, index_naming + '.p'])) / (1024. ** 2)
                pstart_size = os.path.getsize(os.sep.join([index_folder, index_naming + '.pstart'])) / (1024. ** 2)
                space_size_lst.append(float(format_str(d_size + p_size + pstart_size)))
        return dict(zip(data_set_lst, space_size_lst))


# 3rd: linear-d
class LinearDIndexingStat:
    # unit: seconds
    @staticmethod
    def get_indexing_time():
        indexing_time_lst = []
        for data_set in data_set_lst:
            for algorithm_name in ['LinSimAP', 'LinSimBench', 'lind-rand-ben']:
                indexing_time = get_tag_info(
                    os.sep.join([other_algo_indexing_stat_root_folder, data_set, algorithm_name + '.txt']),
                    tag='indexing time')

                if indexing_time is not None:
                    indexing_time_lst.append(float(format_str(indexing_time)))
                    break
        return dict(zip(data_set_lst, indexing_time_lst))

    # unit: MB
    @staticmethod
    def get_index_disk_size():
        index_folder = os.sep.join([datasets_root_folder, 'linearD'])
        space_size_lst = []
        for data_set in data_set_lst:
            index_naming = '-'.join([data_set, '0.600', '10', '3', '100'])
            d_size = os.path.getsize(os.sep.join([index_folder, index_naming + '.D'])) / (1024. ** 2)
            P_size = os.path.getsize(os.sep.join([index_folder, data_set + '.P'])) / (1024. ** 2)
            PT_size = os.path.getsize(os.sep.join([index_folder, data_set + '.PT'])) / (1024. ** 2)
            space_size_lst.append(float(format_str(d_size + P_size + PT_size)))
        return dict(zip(data_set_lst, space_size_lst))


# 4th: cloud-walker
class CloudWalkerIndexingStat:
    # unit: seconds
    @staticmethod
    def get_indexing_time():
        indexing_time_lst = []
        for data_set in data_set_lst:
            for algorithm_name in ['CloudWalkerAP', 'CloudWalkerBench', 'cw-rand-gen']:
                indexing_time = get_tag_info(
                    os.sep.join([other_algo_indexing_stat_root_folder, data_set, algorithm_name + '.txt']),
                    tag='indexing time')

                if indexing_time is not None:
                    indexing_time_lst.append(float(format_str(indexing_time)))
                    break
        return dict(zip(data_set_lst, indexing_time_lst))

    # unit: MB
    @staticmethod
    def get_index_disk_size():
        index_folder = os.sep.join([datasets_root_folder, 'cloudwalker'])
        space_size_lst = []
        for data_set in data_set_lst:
            index_naming = '-'.join([data_set, '0.600', '10', '3', '100', '10000'])
            d_size = os.path.getsize(os.sep.join([index_folder, index_naming + '.D'])) / (1024. ** 2)
            space_size_lst.append(float(format_str(d_size)))
        return dict(zip(data_set_lst, space_size_lst))


# 5th: tsf
class TSFIndexingStat:
    # unit: seconds
    @staticmethod
    def get_indexing_time():
        indexing_time_lst = []
        for data_set in data_set_lst:
            for algorithm_name in ['tsf-ap', 'tsf-bench', 'tsf-rand-bench']:
                indexing_time = get_tag_info(
                    os.sep.join([other_algo_indexing_stat_root_folder, data_set, algorithm_name + '.txt']),
                    tag='indexing computation time')

                if indexing_time is not None:
                    indexing_time_lst.append(float(format_str(indexing_time)))
                    break
        return dict(zip(data_set_lst, indexing_time_lst))

    # unit: MB
    @staticmethod
    def get_index_disk_size():
        sample_one_way_graph_num = 100
        space_size_lst = []
        for data_set in data_set_lst:
            space_size_lst.append(
                float(format_str(sample_one_way_graph_num * size_of_int * v_num_dict[data_set] / (1024. ** 2))))
        return dict(zip(data_set_lst, space_size_lst))


if __name__ == '__main__':
    def get_dict(algorithm_obj):
        assert isinstance(algorithm_obj, LocalPushIndexingStat) or isinstance(algorithm_obj, SlingIndexingStat) or \
               isinstance(algorithm_obj, LinearDIndexingStat) or isinstance(algorithm_obj, CloudWalkerIndexingStat) \
               or isinstance(algorithm_obj, TSFIndexingStat)
        ret_dict = {
            index_time_tag: algorithm_obj.get_indexing_time(),
            index_size_tag: algorithm_obj.get_index_disk_size(),
        }
        return ret_dict


    algorithm_tag_lst = [
        local_push_tag, sling_tag,
        linear_d_tag, cloud_walker_tag,
        tsf_tag
    ]
    algorithm_obj_lst = [
        LocalPushIndexingStat(), SlingIndexingStat(),
        LinearDIndexingStat(), CloudWalkerIndexingStat(),
        TSFIndexingStat()
    ]
    index_info_dict = dict(zip(algorithm_tag_lst, map(get_dict, algorithm_obj_lst)))
    my_str = json.dumps(index_info_dict, indent=4)
    os.system('mkdir -p data-json')
    with open('data-json/index_result.json', 'w') as ofs:
        ofs.write(my_str)
