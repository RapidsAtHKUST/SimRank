import commands
import sys
import time

import os

import time_out_util

my_splitter = '-'.join(['*' for _ in xrange(20)])


def kill_all():
    # kill *-bench
    exec_name_lst = []
    for exec_name in exec_name_lst:
        err_code, output = commands.getstatusoutput("ps -ef | grep " + exec_name + " | awk '{print $2}'")
        for pid in output.strip().split('\n'):
            os.system('kill -9 ' + pid)
    time.sleep(5)


def write_split(statistics_file_path):
    with open(statistics_file_path, 'a+') as ifs:
        ifs.write(my_splitter + my_splitter + '\n')
        ifs.write(my_splitter + my_splitter + '\n')


def signal_handler(signal, frame):
    print 'You pressed Ctrl+C!'
    kill_all()
    sys.exit(0)


def run_exp():
    other_exec_path_lst = [
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/sling/build/sling-rand-bench-gt',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/isp-yche/build/isp-rand-bench-gt',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/LinearD/build/lind-rand-ben-gt',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/LinearD/build/cw-rand-gen-gt',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/tsf/build/tsf-rand-bench-gt'
    ]
    data_set_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote']
    # folder_name = 'other_methods_overview_01_17'
    folder_name = 'other_methods_overview_03_29'
    tag = 'exp_results'
    range_idx_lst = range(10)

    # sample_num_lst= [10 ** 3, 10 ** 4, 10 ** 5, 10 ** 6]
    sample_num_lst = [10 ** 3, 10 ** 4]

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
                        time_out = 7200

                        tle_flag, info, correct_info = time_out_util.run_with_timeout(cmd, timeout_sec=time_out)
                        write_split(statistics_file_path)

                        with open(statistics_file_path, 'a+') as ifs:
                            ifs.write(correct_info)
                            ifs.write(my_splitter + time.ctime() + my_splitter)
                            ifs.write('is_time_out:' + str(tle_flag))
                            ifs.write('\n\n\n\n')
                        print 'finish:', cmd

                        if tle_flag:
                            is_cur_sample_scale_tle = True
                            break
                    if is_cur_sample_scale_tle:
                        break

    # one_round()

    # sample_num_lst = [10 ** 3, 10 ** 4, 10 ** 5]
    # sample_num_lst = [10 ** 3, 10 ** 4]
    sample_num_lst = [10 ** 3]
    # sample_num_lst = [10 ** 6]
    range_idx_lst = range(1)
    data_set_lst = [
        # 'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        # 'email-Enron', 'email-EuAll', 'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google',
        # 'cit-Patents',
        # 'soc-LiveJournal1'
        'wiki-Link'
    ]

    other_exec_path_lst = [
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/sling/build/sling-rand-bench',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/isp-yche/build/isp-rand-bench',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/LinearD/build/lind-rand-ben',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/LinearD/build/cw-rand-gen',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/tsf/build/tsf-rand-bench'
    ]
    one_round()


if __name__ == '__main__':
    run_exp()
