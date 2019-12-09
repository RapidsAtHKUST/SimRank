import commands
import sys
import time
import os
import time_out_util

my_splitter = '-'.join(['*' for _ in xrange(20)])

def write_split(stat_file_path):
    with open(stat_file_path, 'a+') as ifs:
        ifs.write(my_splitter + my_splitter + '\n')
        ifs.write(my_splitter + my_splitter + '\n')

def run_exp():
    exec_path_lst = [
            # '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/sling/build/sling-topk',
            # '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-fgi/build/blpmc',
            '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-fgi/build/carmo',
            '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-fgi/build/carmo-h',
            '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-fgi/build/carmo-t']
    small_dataset_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06']
    large_dataset_lst = [
            'web-Stanford',
            'web-BerkStan',
            'web-Google',
            'cit-Patents',
            'soc-LiveJournal1',
            'wiki-Link']
    tag = 'exp_results'
    folder_name = 'small_10round'

    for dataset_name in small_dataset_lst:
        for q in [10 ** 5]:
            for k in xrange(100, 501, 100):
                for algo_path in exec_path_lst:
                    for round_idx in xrange(1):
                        for idx in xrange(1):
                            algo_name = algo_path.split('/')[-1]
                            stat_dir = os.sep.join(map(str, ['.', tag, folder_name, q, k, dataset_name]))
                            os.system('mkdir -p ' + stat_dir)
                            stat_file_path = stat_dir + os.sep + algo_name + '.txt'

                            if round_idx == 0:
                                os.system(' '.join(['echo', my_splitter + time.ctime() + my_splitter, '>', stat_file_path]))
                            else:
                                os.system(' '.join(['echo', my_splitter + time.ctime() + my_splitter, '>>', stat_file_path]))

                            params_lst = map(str, [algo_path, dataset_name, q, round_idx, k, '>>', stat_file_path])
                            cmd = ' '.join(params_lst)

                            time_out = 36000
                            tle_flag, info, correct_info = time_out_util.run_with_timeout(cmd, timeout_sec=time_out)
                            write_split(stat_file_path)
                            with open(stat_file_path, 'a+') as ifs:
                                ifs.write(correct_info)
                                ifs.write(my_splitter + time.ctime() + my_splitter)
                                ifs.write('is_time_out:' + str(tle_flag))
                                ifs.write('\n\n\n\n')
                            print 'finish: ', cmd

                            if tle_flag:
                                print 'unable to finish due to timeout', cmd
                                break

if __name__ == '__main__':
    run_exp()
