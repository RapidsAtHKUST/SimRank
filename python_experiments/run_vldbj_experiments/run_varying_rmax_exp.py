from exec_utilities import time_out_util
from exec_utilities.exec_utils import *


def run_varying_eps_exp():
    exec_path_lst = [
        '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/flpmc-rand-rmax',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/bflpmc-rand-rmax',
    ]
    sample_num_lst = [10 ** 6,
                      10 ** 6]
    sample_num_dict = dict(zip(exec_path_lst, sample_num_lst))

    tag = 'exp_results'
    folder_name = 'varying_rmax_exp'
    data_set_name = 'ca-GrQc'
    round_idx = 0

    # algorithm parameters
    rmax_lst = list(([0.01 * (i + 1) for i in range(30)]))

    def one_round():
        # algorithm by algorithm to finish the experiment
        for algorithm_path in exec_path_lst:
            for rmax in rmax_lst:
                algorithm = algorithm_path.split('/')[-1]
                statistics_dir = os.sep.join(
                    map(str, ['.', tag, folder_name, data_set_name, rmax]))
                os.system('mkdir -p ' + statistics_dir)
                statistics_file_path = statistics_dir + os.sep + algorithm + '.txt'

                # 1st: write header
                os.system(' '.join(
                    ['echo', my_splitter + time.ctime() + my_splitter, '>>', statistics_file_path]))
                params_lst = map(str,
                                 [algorithm_path, data_set_name, sample_num_dict[algorithm_path], round_idx, rmax, '>>',
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

                # 3rd: if tle, break
                if tle_flag:
                    break

    one_round()


if __name__ == '__main__':
    run_varying_eps_exp()
