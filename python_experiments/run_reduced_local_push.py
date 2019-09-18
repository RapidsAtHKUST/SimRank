from exec_utilities import time_out_util
from exec_utilities.exec_utils import *


def run_exp():
    our_exec_path = '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build'

    data_set_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
                    'email-Enron', 'email-EuAll', 'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google',
                    'cit-Patents', 'soc-LiveJournal1']
    our_algorithm = 'rlp'

    def one_round():
        for data_set_name in data_set_lst:
            algorithm_path = our_exec_path + os.sep + our_algorithm
            statistics_file_path = 'exp_results/' + 'rlp_dynamic.txt'
            params_lst = map(str, [algorithm_path, data_set_name, '>>', statistics_file_path])
            cmd = ' '.join(params_lst)
            time_out = 1000 if data_set_name != 'soc-LiveJournal1' else 3600

            tle_flag, info, correct_info = time_out_util.run_with_timeout(cmd, timeout_sec=time_out)
            write_split(statistics_file_path)

            with open(statistics_file_path, 'a+') as ifs:
                ifs.write(correct_info)
                ifs.write(my_splitter + time.ctime() + my_splitter)
                ifs.write('is_time_out:' + str(tle_flag))
                ifs.write('\n\n\n\n')
            print('finish:', cmd)

    one_round()


if __name__ == '__main__':
    run_exp()
