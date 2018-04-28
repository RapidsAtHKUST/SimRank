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
    our_exec_path = '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Parallel-Profile/build-icc'

    data_set_lst = [
        'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        'email-Enron', 'email-EuAll',
        'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google',
        'cit-Patents',
        'soc-LiveJournal1'
    ]
    our_algorithm_lst = [
        # 'prlp-with-lock',
        'pflp-with-lock',
        'prlp-lock-free',
        # 'prlp-lock-free-stat',
        # 'prlp-with-lock-stat',
    ]
    thread_num_lst = [8, 16, 32, 56, 64]

    # thread_num_lst = [56]

    def one_round():
        for data_set_name in data_set_lst:
            for our_algorithm in our_algorithm_lst:
                statistics_folder_path = os.sep.join(['plp_scalability_results_04_24', our_algorithm, data_set_name])
                os.system('mkdir -p ' + statistics_folder_path)

                for thread_num in list(reversed(thread_num_lst)):
                    # os.system('export OMP_NUM_THREADS=' + str(thread_num))

                    algorithm_path = os.sep.join([our_exec_path, our_algorithm])
                    statistics_file_path = os.sep.join([statistics_folder_path, str(thread_num) + ".txt"])

                    # params_lst = map(str, [algorithm_path, data_set_name, 0.01, 'save', '>>', statistics_file_path])
                    params_lst = map(str, [algorithm_path, data_set_name, 0.01, '>>', statistics_file_path])
                    cmd = ' '.join(params_lst)
                    # time_out = 3600 if data_set_name != 'soc-LiveJournal1' else 7200
                    time_out = 36000

                    my_env = os.environ.copy()
                    my_env["OMP_NUM_THREADS"] = str(thread_num)
                    tle_flag, info, correct_info = time_out_util.run_with_timeout(cmd, timeout_sec=time_out, env=my_env)
                    write_split(statistics_file_path)

                    with open(statistics_file_path, 'a+') as ofs:
                        ofs.write(correct_info)
                        ofs.write(my_splitter + time.ctime() + my_splitter)
                        ofs.write('is_time_out:' + str(tle_flag))
                        ofs.write('\n\n\n\n')
                    print 'finish:', cmd

    for i in xrange(5):
        one_round()

    # our_algorithm_lst = ['rlp', 'flp']
    # thread_num_lst = [1]
    # one_round()


if __name__ == '__main__':
    run_exp()
