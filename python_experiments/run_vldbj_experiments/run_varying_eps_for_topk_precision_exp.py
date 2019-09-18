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
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/bprw-rand-bench-gt',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/flpmc-rand-bench-gt',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/bflpmc-rand-bench-gt',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/sling/build/sling-rand-bench-gt',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/isp-yche/build/isp-rand-bench-gt',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/tsf/build/tsf-rand-bench-gt',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/READS/build/reads-d-rand-bench',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/READS/build/reads-rq-rand-bench',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/LinearD/build/lind-rand-ben-gt',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/LinearD/build/cw-rand-gen-gt',
        '/homes/ywangby/workspace/yche/new-git-repos-yche/SimRank/SPS-Variants/ProbeSim_vldb_pub/build/ProbeSim-gt'
    ]

    tag = 'exp_results'
    folder_name = 'varying_eps_for_topk_precision_exp'
    sample_num = str(10 ** 4)
    k = str(800)
    data_set_name_lst = [
        'ca-GrQc',
        'ca-HepTh',
        'p2p-Gnutella06',
        'wiki-Vote'
    ]
    round_idx = 0

    # algorithm parameters
    def one_round():
        # algorithm by algorithm to finish the experiment
        for algorithm_path in exec_path_lst:
            for data_set_name in data_set_name_lst:
                eps_lst = list(reversed([0.0001, 0.0004, 0.0016, 0.0064, 0.0256]))[2:3]
                if 'sling' in algorithm_path or 'reads-d' in algorithm_path:
                    eps_lst = list(reversed([0.0016, 0.0064, 0.0256]))
                elif 'tsf' in algorithm_path:
                    eps_lst = list(reversed([0.0064, 0.0256]))

                for eps in eps_lst:
                    algorithm = algorithm_path.split('/')[-1]
                    statistics_dir = os.sep.join(map(str, ['.', tag, folder_name, sample_num, k, data_set_name, eps]))
                    os.system('mkdir -p ' + statistics_dir)
                    statistics_file_path = statistics_dir + os.sep + algorithm + '.txt'

                    # 1st: write header
                    os.system(' '.join(['echo', my_splitter + time.ctime() + my_splitter, '>>', statistics_file_path]))
                    if algorithm_path.endswith('reads-rq-rand-bench'):
                        eps = max(0.008, eps)
                    params_lst = map(str, [algorithm_path, data_set_name, sample_num, round_idx, k, eps, '>>',
                                           statistics_file_path])
                    if algorithm_path.split('/')[-1] in ['lind-rand-ben-gt', 'cw-rand-gen-g']:
                        params_lst = map(str, [algorithm_path, data_set_name, sample_num, round_idx, k, '>>',
                                               statistics_file_path])
                    if algorithm_path.endswith('ProbeSim-gt'):
                        params_lst = map(str, [algorithm_path, data_set_name, sample_num, round_idx, 0.6, eps,
                                               0.01, k, '>>', statistics_file_path])

                    cmd = ' '.join(params_lst)
                    print cmd
                    time_out = 7200

                    # 2nd: run cmd
                    tle_flag, info, correct_info = time_out_util.run_with_timeout(cmd, timeout_sec=time_out)
                    write_split(statistics_file_path)
                    with open(statistics_file_path, 'a+') as ifs:
                        ifs.write(correct_info)
                        ifs.write(my_splitter + time.ctime() + my_splitter)
                        ifs.write('is_time_out:' + str(tle_flag))
                        ifs.write('\n\n\n\n')
                    print 'finish:', cmd

                    # if lind or cw, break
                    if algorithm_path.split('/')[-1] in ['lind-rand-ben-gt', 'cw-rand-gen-g']:
                        break
                    # 3rd: if tle, break
                    if tle_flag:
                        print 'too much time, not able to finish', cmd
                        break

    one_round()


if __name__ == '__main__':
    run_varying_eps_exp()
