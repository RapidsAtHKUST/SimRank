import decimal

import os

data_set_lst = [
    'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
    'email-Enron', 'email-EuAll', 'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google',
    'cit-Patents', 'soc-LiveJournal1']

our_algo_indexing_stat_root_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/' \
                                     'yche/git-repos/SimRank/python_experiments/exp_results/' \
                                     'our_methods_overview_01_16'

other_algo_indexing_stat_root_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/' \
                                       'yche/git-repos/SimRank/python_experiments/exp_results/' \
                                       'other_methods_overview_01_16'

datasets_root_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/LinsysSimRank/datasets/'


def format_str(float_num):
    return str(decimal.Decimal.from_float(float_num).quantize(decimal.Decimal('0.000')))


def ms_val_to_s(num):
    return eval(str(decimal.Decimal.from_float(float(num) / 1000).quantize(decimal.Decimal('0.0000'))))


def get_tag_info(file_path, tag):
    if not os.path.exists(file_path):
        return None
    with open(file_path) as ifs:
        # assume unit seconds
        lst = map(lambda line: eval(line.split(':')[-1].split('s')[0]),
                  filter(lambda line: line.startswith(tag), ifs.readlines()))
        return None if len(lst) == 0 else min(lst)


# our full local push related
class LocalPushIndexingStat:
    # unit: seconds
    @staticmethod
    def get_indexing_time():
        indexing_time_lst = []
        for data_set in data_set_lst:
            for algorithm_name in ['flpmc-ap', 'flpmc-bench']:
                indexing_time = get_tag_info(
                    os.sep.join([our_algo_indexing_stat_root_folder, data_set, algorithm_name + '.txt']),
                    tag='total indexing cost')
                if indexing_time is not None:
                    indexing_time_lst.append(indexing_time)
                    break
        return dict(zip(data_set_lst, indexing_time_lst))
        # return zip(data_set_lst, indexing_time_lst)

    # unit: MB
    @staticmethod
    def get_mem_size():
        local_push_folder = os.sep.join([datasets_root_folder, 'local_push'])
        mem_size_lst = []
        for data_set in data_set_lst:
            index_naming = '_'.join(['FLP', '-'.join([data_set, '0.600', '0.116040'])])
            with open(os.sep.join([local_push_folder, index_naming + '.meta'])) as ifs:
                mem_size_lst.append(float(format_str(float(ifs.readlines()[5].strip()) / 1024.)))
        return dict(zip(data_set_lst, mem_size_lst))
        # return zip(data_set_lst, mem_size_lst)

    # unit: MB
    @staticmethod
    def get_index_disk_size():
        local_push_folder = os.sep.join([datasets_root_folder, 'local_push'])
        space_size_lst = []
        for data_set in data_set_lst:
            index_naming = '_'.join(['FLP', '-'.join([data_set, '0.600', '0.116040'])])
            p_size = os.path.getsize(os.sep.join([local_push_folder, index_naming + '.P'])) / (1024. ** 2)
            r_size = os.path.getsize(os.sep.join([local_push_folder, index_naming + '.R'])) / (1024. ** 2)
            space_size_lst.append(float(format_str(p_size + r_size)))
        return dict(zip(data_set_lst, space_size_lst))


if __name__ == '__main__':
    my_str = str()
    print LocalPushIndexingStat.get_indexing_time()
    print LocalPushIndexingStat.get_mem_size()
    print LocalPushIndexingStat.get_index_disk_size()
