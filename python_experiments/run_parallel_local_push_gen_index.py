from exec_utilities import time_out_util
from exec_utilities.exec_utils import *


def run_exp():
    our_exec_path = '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Parallel-Profile/build'

    data_set_lst = [
        'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        'email-Enron', 'email-EuAll', 'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google',
        'cit-Patents', 'soc-LiveJournal1',
        'wiki-Link'
    ]
    # our_algorithm_lst = ['pflp', 'prlp']
    our_algorithm_lst = ['prlp']
    thread_num_lst = [56]

    # thread_num_lst = [1]

    def one_round():
        for data_set_name in data_set_lst:
            for our_algorithm in our_algorithm_lst:
                statistics_folder_path = os.sep.join(['plp_parallel_gen_idx_0405', our_algorithm, data_set_name])
                os.system('mkdir -p ' + statistics_folder_path)

                for thread_num in list(reversed(thread_num_lst)):
                    # os.system('export OMP_NUM_THREADS=' + str(thread_num))

                    algorithm_path = os.sep.join([our_exec_path, our_algorithm])
                    statistics_file_path = os.sep.join([statistics_folder_path, str(thread_num) + ".txt"])

                    params_lst = map(str, [algorithm_path, data_set_name, 0.116040, 'save', '>>', statistics_file_path])
                    cmd = ' '.join(params_lst)
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

    one_round()


if __name__ == '__main__':
    run_exp()
