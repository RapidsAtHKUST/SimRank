from exec_utilities import time_out_util
from exec_utilities.exec_utils import *


def run_exp():
    other_exec_path_lst = [
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/sling/build/sling_all',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/isp-yche/build/isp_ap',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/LinearD/build/LinSimAP',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/LinearD/build/CloudWalkerAP',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/tsf/build/tsf-ap'
    ]
    data_set_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote']
    folder_name = 'other_methods_overview'
    tag = 'exp_results'

    def one_round():
        for data_set_name in data_set_lst:
            for other_algorithm_path in other_exec_path_lst:
                statistics_dir = os.sep.join(map(str, ['.', tag, folder_name, data_set_name]))
                os.system('mkdir -p ' + statistics_dir)
                statistics_file_path = statistics_dir + os.sep + other_algorithm_path.split('/')[-1] + '.txt'
                # 1st: write header
                os.system(' '.join(
                    ['echo', my_splitter + time.ctime() + my_splitter, '>>', statistics_file_path]))
                params_lst = map(str, [other_algorithm_path, data_set_name, '>>', statistics_file_path])
                cmd = ' '.join(params_lst)
                time_out = 1800 if data_set_name != 'soc-LiveJournal1' else 3600

                tle_flag, info, correct_info = time_out_util.run_with_timeout(cmd, timeout_sec=time_out)
                write_split(statistics_file_path)

                with open(statistics_file_path, 'a+') as ifs:
                    ifs.write(correct_info)
                    ifs.write(my_splitter + time.ctime() + my_splitter)
                    ifs.write('is_time_out:' + str(tle_flag))
                    ifs.write('\n\n\n\n')

    # 1st: verify err
    one_round()
    # 2nd: measure efficiency
    data_set_lst = [
        'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        'email-Enron', 'email-EuAll',
        'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google', 'cit-Patents',
        'soc-LiveJournal1']
    other_exec_path_lst = [
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/sling/build/sling_bench',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/isp-yche/build/isp_bench',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/LinearD/build/LinSimBench',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/LinearD/build/CloudWalkerBench',
        '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/tsf/build/tsf-bench'
    ]
    one_round()


if __name__ == '__main__':
    run_exp()
