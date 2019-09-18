import datetime

from exec_utilities import time_out_util
from exec_utilities.exec_utils import *


def run_exp():
    our_exec_path = '/homes/ywangby/workspace/yche/git-repos/SimRank/APS/build'

    data_set_lst = [
        # 'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        # 'email-Enron', 'email-EuAll', 'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google',
        # 'cit-Patents', 'soc-LiveJournal1',
        # 'wiki-Link',
        'digg-friends',
        'flickr-growth',
    ]
    our_algorithm = 'dynamic-rlp'

    # insert_edge_num = 100

    def one_round(bool_val):
        timestamp_str = str(datetime.datetime.now()).split('.')[0].replace(' ', '-').replace(':', '-')
        name_str = 'flp_dynamic_update_time_' if 'rlp' in our_algorithm \
            else 'flp_dynamic_update_time_'
        statistics_file_path = 'exp_results/' + name_str + str(insert_edge_num) + '_' + timestamp_str + '.txt'

        for data_set_name in data_set_lst:
            algorithm_path = our_exec_path + os.sep + our_algorithm
            params_lst = map(str, [algorithm_path, data_set_name, insert_edge_num, 'ins' if bool_val == True else 'del',
                                   '>>', statistics_file_path])
            cmd = ' '.join(params_lst)
            # time_out = 1000 if data_set_name != 'soc-LiveJournal1' else 3600
            time_out = 72000
            tle_flag, info, correct_info = time_out_util.run_with_timeout(cmd, timeout_sec=time_out)
            write_split(statistics_file_path)

            with open(statistics_file_path, 'a+') as ifs:
                ifs.write(correct_info)
                ifs.write(my_splitter + time.ctime() + my_splitter)
                ifs.write('is_time_out:' + str(tle_flag))
                ifs.write('\n\n\n\n')

    # one_round()
    insert_edge_num = 1000
    # one_round()

    our_algorithm = 'dynamic-flp'
    one_round(True)
    one_round(False)


if __name__ == '__main__':
    run_exp()
