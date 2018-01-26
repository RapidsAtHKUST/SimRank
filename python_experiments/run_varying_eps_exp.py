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


def run_varying_eps_exp():
    exec_path_lst = [
        '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/bprw-rand-varying',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/flpmc-rand-varying',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/bflpmc-rand-varying',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/sling/build/sling-rand-varying',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/isp-yche/build/isp-rand-varying',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/tsf/build/tsf-rand-varying',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/READS/build/reads-d-rand-bench',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/READS/build/reads-rq-rand-bench',
    ]
    sample_num_lst = [10 ** 6,
                      10 ** 6,
                      10 ** 6,
                      10 ** 6,
                      10 ** 5,
                      10 ** 3,
                      10 ** 5,
                      10 ** 5]
    sample_num_dict = dict(zip(exec_path_lst, sample_num_lst))
    tag = 'exp_results'
    folder_name = 'varying_eps_exp'
    data_set_name = 'ca-GrQc'
    round_idx = 0

    # algorithm parameters
    c = 0.6
    delta = 0.01
    # eps_lst = list(reversed([0.001 * (i + 1) for i in xrange(100)]))
    eps_lst = list(([0.001 * (i + 1) for i in xrange(30)]))

    exec_path_lst = [
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/bprw-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/flpmc-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/bflpmc-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/sling/build/sling-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/isp-yche/build/isp-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/tsf/build/tsf-rand-varying'
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/READS/build/reads-d-rand-bench',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/READS/build/reads-rq-rand-bench'
    ]

    def one_round():
        # algorithm by algorithm to finish the experiment
        for algorithm_path in exec_path_lst:
            for eps in eps_lst:
                algorithm = algorithm_path.split('/')[-1]
                statistics_dir = os.sep.join(
                    map(str, ['.', tag, folder_name, data_set_name, sample_num_dict[algorithm_path], eps]))
                os.system('mkdir -p ' + statistics_dir)
                statistics_file_path = statistics_dir + os.sep + algorithm + '.txt'

                # 1st: write header
                os.system(' '.join(
                    ['echo', my_splitter + time.ctime() + my_splitter, '>>', statistics_file_path]))
                params_lst = map(str,
                                 [algorithm_path, data_set_name, sample_num_dict[algorithm_path], round_idx, c, eps,
                                  delta, '>>',
                                  statistics_file_path])
                cmd = ' '.join(params_lst)
                time_out = 1200

                # 2nd: run cmd
                tle_flag, info, correct_info = time_out_util.run_with_timeout(cmd, timeout_sec=time_out)
                write_split(statistics_file_path)
                with open(statistics_file_path, 'a+') as ifs:
                    ifs.write(correct_info)
                    ifs.write(my_splitter + time.ctime() + my_splitter)
                    ifs.write('is_time_out:' + str(tle_flag))
                    ifs.write('\n\n\n\n')
                print 'finish:', cmd

                # 3rd: if tle, break
                if tle_flag:
                    print 'too much time, not able to finish', cmd
                    break

    one_round()


if __name__ == '__main__':
    run_varying_eps_exp()
