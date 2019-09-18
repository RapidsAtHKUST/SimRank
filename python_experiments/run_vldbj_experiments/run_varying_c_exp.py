from exec_utilities import time_out_util
from exec_utilities.exec_utils import *


def run_exp():
    exec_path_lst = [
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/bprw-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/flpmc-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/bflpmc-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/sling/build/sling-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/isp-yche/build/isp-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/tsf/build/tsf-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/READS/build/reads-d-rand-bench',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/READS/build/reads-rq-rand-bench',
        '/homes/ywangby/workspace/yche/new-git-repos-yche/SimRank/SPS-Variants/ProbeSim_vldb_pub/build/ProbeSim'
    ]
    sample_num_lst = [
        # 10 ** 6,
        # 10 ** 6,
        # 10 ** 6,
        # 10 ** 6,
        # 10 ** 5,
        # 10 ** 3,
        # 10 ** 5,
        # 10 ** 5,
        10 ** 3
    ]
    sample_num_dict = dict(zip(exec_path_lst, sample_num_lst))

    tag = 'exp_results'
    folder_name = 'varying_c_exp'
    data_set_name = 'ca-GrQc'
    round_idx = 0

    # algorithm parameters
    c_lst = list(([0.1 * (i + 1) for i in range(8)]))
    delta = 0.01
    eps = 0.01

    def one_round():
        # algorithm by algorithm to finish the experiment
        for algorithm_path in exec_path_lst:
            for c in c_lst:
                algorithm = algorithm_path.split('/')[-1]
                statistics_dir = os.sep.join(
                    map(str, ['.', tag, folder_name, data_set_name, sample_num_dict[algorithm_path], c]))
                os.system('mkdir -p ' + statistics_dir)
                statistics_file_path = statistics_dir + os.sep + algorithm + '.txt'

                # 1st: write header
                os.system(' '.join(
                    ['echo', my_splitter + time.ctime() + my_splitter, '>>', statistics_file_path]))
                params_lst = map(str,
                                 [algorithm_path, data_set_name, sample_num_dict[algorithm_path], round_idx, c, eps,
                                  delta, 200, '>>', statistics_file_path])
                cmd = ' '.join(params_lst)
                time_out = 3600

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

    exec_path_lst = [
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/bprw-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/flpmc-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/bflpmc-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/sling/build/sling-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/isp-yche/build/isp-rand-varying',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/tsf/build/tsf-rand-varying'
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/READS/build/reads-d-rand-bench',
        # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/READS/build/reads-rq-rand-bench',
        '/homes/ywangby/workspace/yche/new-git-repos-yche/SimRank/SPS-Variants/ProbeSim_vldb_pub/build/ProbeSim'
    ]
    one_round()


if __name__ == '__main__':
    run_exp()
