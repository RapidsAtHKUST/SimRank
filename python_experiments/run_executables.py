import time

import os

import time_out_util

if __name__ == '__main__':
    any_scan_path = '/ghome/yche/projects/anySCANBin/anyscan1121'
    data_set_lst = [
        "ca-HepPh", "ca-GrQc", "p2p-Gnutella06", "wiki-Vote",
        "web-NotreDame", "web-Stanford", "web-BerkStan", "web-Google",
        "soc-LiveJournal1",
    ]
    parameter_eps_lst = [float(i + 1) / 10 for i in xrange(9)]
    parameter_min_pts_lst = [5]

    data_set_lst = map(lambda name: os.pardir + os.sep + 'dataset' + os.sep + name, data_set_lst)
    thread_num = 64
    foler_name = 'anyscan-exp-1121'
    for data_set_path in data_set_lst:
        for eps in parameter_eps_lst:
            for min_pts in parameter_min_pts_lst:
                # statistics output dir and file
                data_set_name = data_set_path.split(os.sep)[-1]
                statistics_dir = os.sep.join(
                    map(str, ['.', foler_name, data_set_name, 'eps-' + str(eps), 'min_pts-' + str(min_pts)]))
                os.system('mkdir -p ' + statistics_dir)
                statistics_file_path = statistics_dir + os.sep + '-'.join(
                    map(str, ['output', data_set_name, eps, min_pts])) + '.txt'

                # header
                my_splitter = '-'.join(['*' for _ in xrange(20)])
                os.system(' '.join(
                    ['echo', my_splitter + time.ctime() + my_splitter, '>>', statistics_file_path]))


                def write_split():
                    with open(statistics_file_path, 'a+') as ifs:
                        ifs.write(my_splitter + my_splitter + '\n')
                        ifs.write(my_splitter + my_splitter + '\n')


                params_lst = map(str, [any_scan_path,
                                       '-c', 4, '-g', 'label.gold',
                                       '-i', data_set_path, '-o', 'out.txt', '-e', eps, '-m', min_pts,
                                       '-a', 65536, '-b', 65536, '-t', thread_num, '>>', statistics_file_path])
                cmd = ' '.join(params_lst)
                print cmd
                tle_flag, info, correct_info = time_out_util.run_with_timeout(cmd, timeout_sec=3600)
                write_split()

                with open(statistics_file_path, 'a+') as ifs:
                    ifs.write(correct_info)
                    ifs.write(my_splitter + time.ctime() + my_splitter)
                    ifs.write('is_time_out:' + str(tle_flag))
                    ifs.write('\n\n\n\n')
                print 'finish:', '-'.join(map(str, [data_set_path, eps, min_pts]))
    print
