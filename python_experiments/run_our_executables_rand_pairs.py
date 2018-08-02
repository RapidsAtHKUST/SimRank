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
    our_exec_path = '/homes/ywangby/workspace/yche/new-git-repos-yche/SimRank/LPMC-Profile/build'
    our_exec_name_lst = ['flpmc-rand-bench-gt', 'bflpmc-rand-bench-gt', 'bprw-rand-bench-gt', ]
    data_set_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote']
    folder_name = 'our_methods_overview_08_02'
    tag = 'exp_results'
    range_idx_lst = range(10)

    def one_round():
        for data_set_name in data_set_lst:
            for our_algorithm in our_exec_name_lst:
                # for sample_num in list(reversed([10 ** 3, 10 ** 4, 10 ** 5, 10 ** 6])):
                for sample_num in [10 ** 6]:
                    is_cur_sample_scale_tle = False
                    for round_idx in range_idx_lst:
                        statistics_dir = os.sep.join(
                            map(str, ['.', tag, folder_name, data_set_name, sample_num, round_idx]))
                        os.system('mkdir -p ' + statistics_dir)
                        statistics_file_path = statistics_dir + os.sep + our_algorithm + '.txt'

                        # 1st: write header
                        os.system(' '.join(
                            ['echo', my_splitter + time.ctime() + my_splitter, '>>', statistics_file_path]))
                        algorithm_path = our_exec_path + os.sep + our_algorithm
                        params_lst = map(str, [algorithm_path, data_set_name, sample_num, round_idx, '>>',
                                               statistics_file_path])
                        cmd = ' '.join(params_lst)
                        time_out = 3600

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

    # range_idx_lst = range(2)
    # range_idx_lst = range(1)
    range_idx_lst = [0]
    data_set_lst = [
        # 'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        # 'email-Enron', 'email-EuAll', 'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google',
        # 'cit-Patents',
        # 'soc-LiveJournal1',
        # 'wiki-Link',
        'digg-friends',
        'flickr-growth',
    ]

    our_exec_name_lst = [
        'flpmc-rand-bench', 'bflpmc-rand-bench',
        'bprw-rand-bench', ]
    one_round()


if __name__ == '__main__':
    run_exp()
