from exec_utilities import time_out_util
from exec_utilities.exec_utils import *


def run_exp():
    our_exec_path = '/homes/ywangby/workspace/yche/new-git-repos-yche/SimRank/SPS-Variants/READS/build'
    data_set_lst = [
        # 'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        # 'email-Enron', 'email-EuAll', 'web-NotreDame', 'web-Stanford',
        # 'web-BerkStan', 'web-Google',
        # 'cit-Patents', 'soc-LiveJournal1',
        # 'wiki-Link',
        'digg-friends',
        'flickr-growth',
    ]
    dynamic_exec_tag_lst = [
        'reads-rq-dynamic-del',
        'reads-rq-dynamic-exp',
        # 'reads-d-dynamic-del',
        # 'reads-d-dynamic-exp',
    ]

    def one_round():
        for exec_name in dynamic_exec_tag_lst:
            for data_set_name in data_set_lst:
                algorithm_path = our_exec_path + os.sep + exec_name
                statistics_file_path = 'exp_results/' + exec_name + '_dynamic_update_time_' + str(
                    insert_edge_num) + '_0407.txt'
                params_lst = map(str, [algorithm_path, data_set_name, '>>', statistics_file_path])
                cmd = ' '.join(params_lst)
                time_out = 72000
                tle_flag, info, correct_info = time_out_util.run_with_timeout(cmd, timeout_sec=time_out)
                write_split(statistics_file_path)

                with open(statistics_file_path, 'a+') as ifs:
                    ifs.write(correct_info)
                    ifs.write(my_splitter + time.ctime() + my_splitter)
                    ifs.write('is_time_out:' + str(tle_flag))
                    ifs.write('\n\n\n\n')

    insert_edge_num = 1000
    one_round()


if __name__ == '__main__':
    run_exp()
