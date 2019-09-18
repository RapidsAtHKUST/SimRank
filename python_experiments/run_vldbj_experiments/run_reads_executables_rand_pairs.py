from exec_utilities import time_out_util
from exec_utilities.exec_utils import *


def run_exp():
    exec_root = '/homes/ywangby/workspace/yche/new-git-repos-yche/SimRank/SPS-Variants/READS/build'
    exec_name_lst = [
        # 'reads-s-rand-bench',
        'reads-d-rand-bench',
        'reads-rq-rand-bench'
    ]
    other_exec_path_lst = map(lambda exec_name: exec_root + os.sep + exec_name, exec_name_lst)

    data_set_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote']
    folder_name = 'reads_08_03_efficiency'
    tag = 'exp_results'
    range_idx_lst = range(10)

    sample_num_lst = [10 ** 5]

    def one_round():
        # algorithm by algorithm to finish the experiment
        for other_algorithm_path in other_exec_path_lst:
            for data_set_name in data_set_lst:

                for sample_num in sample_num_lst:
                    is_cur_sample_scale_tle = False
                    for round_idx in range_idx_lst:
                        other_algorithm = other_algorithm_path.split('/')[-1]
                        statistics_dir = os.sep.join(
                            map(str, ['.', tag, folder_name, data_set_name, sample_num, round_idx]))
                        os.system('mkdir -p ' + statistics_dir)
                        statistics_file_path = statistics_dir + os.sep + other_algorithm + '.txt'

                        # 1st: write header
                        os.system(' '.join(
                            ['echo', my_splitter + time.ctime() + my_splitter, '>>', statistics_file_path]))
                        params_lst = map(str, [other_algorithm_path, data_set_name, sample_num, round_idx, '>>',
                                               statistics_file_path])
                        cmd = ' '.join(params_lst)
                        # time_out = 1200 if data_set_name != 'soc-LiveJournal1' else 3600
                        time_out = 36000

                        tle_flag, info, correct_info = time_out_util.run_with_timeout(cmd, timeout_sec=time_out)
                        write_split(statistics_file_path)

                        with open(statistics_file_path, 'a+') as ifs:
                            ifs.write(correct_info)
                            ifs.write(my_splitter + time.ctime() + my_splitter)
                            ifs.write('is_time_out:' + str(tle_flag))
                            ifs.write('\n\n\n\n')

                        if tle_flag:
                            is_cur_sample_scale_tle = True
                            break
                    if is_cur_sample_scale_tle:
                        break

    # 1st: accuracy
    # one_round()

    sample_num_lst = [
        # 10 ** 4,
        10 ** 5
    ]
    range_idx_lst = [0]
    data_set_lst = [
        # 'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        # 'email-Enron', 'email-EuAll',
        # 'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google',
        # 'cit-Patents',
        # 'soc-LiveJournal1',
        # 'wiki-Link',
        'digg-friends',
        'flickr-growth',
    ]

    # 2nd: efficiency
    one_round()

    # 3rd: another round for reads-rq
    # exec_name_lst = ['reads-rq-rand-bench']
    # other_exec_path_lst = map(lambda exec_name: exec_root + os.sep + exec_name, exec_name_lst)
    # # os.system('ll /homes/ywangby/workspace/LinsysSimRank/datasets/readsd/* >> tmp_ll.log')
    # # os.system('du -b /homes/ywangby/workspace/LinsysSimRank/datasets/readsd/* >> tmp_du.log')
    # # os.system('rm /homes/ywangby/workspace/LinsysSimRank/datasets/readsd/* >> tmp_rm.log')
    # data_set_lst = [
    #     'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
    #     'email-Enron', 'email-EuAll',
    #     'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google',
    #     'cit-Patents',
    #     'soc-LiveJournal1'
    # ]
    # sample_num_lst = [
    #     10 ** 4,
    #     10 ** 5
    # ]
    # one_round()


if __name__ == '__main__':
    run_exp()
