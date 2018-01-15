import commands
import os
import sys
import time


def kill_all():
    err_code, output = commands.getstatusoutput("ps -ef | grep cuda | awk '{print $2}'")

    for pid in output.strip().split('\n'):
        os.system('kill -9 ' + pid)
    time.sleep(5)


def signal_handler(signal, frame):
    print 'You pressed Ctrl+C!'
    kill_all()
    sys.exit(0)


def ground_truth_exp():
    our_exec_path = '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/'
    our_ap_exec_name_lst = ['bprw-ap', 'flpmc-ap', 'bflpmc-ap']
    data_set_lst = ["ca-HepPh", "ca-GrQc", "p2p-Gnutella06", "wiki-Vote"]
    folder_name = 'our_methods_overview'

    for dataset in data_set_lst:
        pass


if __name__ == '__main__':
    pass
# data_set_lst = [
#     "ca-HepPh", "ca-GrQc", "p2p-Gnutella06", "wiki-Vote",
#     "web-NotreDame", "web-Stanford", "web-BerkStan", "web-Google",
#     "soc-LiveJournal1",
# ]
#
# our_exec_path = '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/'
#
# our_benchmark_exec_name_lst = ['bprw-bench', 'flpmc-bench', 'bflpmc-bench']
#
# for data_set_name in data_set_lst:
#     statistics_dir = os.sep.join(map(str, ['.', folder_name, data_set_name]))
#     os.system('mkdir -p ' + statistics_dir)
#
#     statistics_file_path = statistics_dir + os.sep + '.txt'
#
#     # header
#     my_splitter = '-'.join(['*' for _ in xrange(20)])
#     os.system(' '.join(
#         ['echo', my_splitter + time.ctime() + my_splitter, '>>', statistics_file_path]))
#
#
#     def write_split():
#         with open(statistics_file_path, 'a+') as ifs:
#             ifs.write(my_splitter + my_splitter + '\n')
#             ifs.write(my_splitter + my_splitter + '\n')
#
#
#     params_lst = map(str, [any_scan_path, '>>', statistics_file_path])
#     cmd = ' '.join(params_lst)
#     print cmd
#     tle_flag, info, correct_info = time_out_util.run_with_timeout(cmd, timeout_sec=1000)
#     write_split()
#
#     with open(statistics_file_path, 'a+') as ifs:
#         ifs.write(correct_info)
#         ifs.write(my_splitter + time.ctime() + my_splitter)
#         ifs.write('is_time_out:' + str(tle_flag))
#         ifs.write('\n\n\n\n')
#     print 'finish:', '-'.join(map(str, [data_set_path, eps, min_pts]))
# print
