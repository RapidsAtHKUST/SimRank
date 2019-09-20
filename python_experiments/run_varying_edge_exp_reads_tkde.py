import socket

from exec_utilities import time_out_util
from config import *
from exec_utilities.exec_utils import *
from multiprocessing import Process


def run_exp(env_tag=lccpu12_tag, with_c_group=True):
    hostname = socket.gethostname()

    with open('config.json') as ifs:
        my_config_dict = json.load(ifs)[env_tag]
    our_exec_path = my_config_dict[exec_path_tag]
    data_set_lst = my_config_dict[data_set_lst_tag]
    thread_num_lst = my_config_dict[thread_num_lst_tag]
    exp_res_root_name = my_config_dict[exp_res_root_path_tag]

    folder_name = 'exp-dynamic-2019-09-20' + os.sep + hostname

    our_exec_name_lst = [
        'reads-rq-dynamic',
        'reads-d-dynamic'
    ]

    work_dir = os.sep.join([exp_res_root_name, folder_name])
    os.system('mkdir -p ' + work_dir)
    logger = get_logger(os.sep.join([work_dir, hostname + '.log']), name=__name__)
    logger.info(my_splitter + time.ctime() + my_splitter)
    logger.info('res folder: {}'.format(folder_name))
    logger.info('our exec folder: {}'.format(our_exec_path))
    logger.info('our exec name list: {}'.format(our_exec_name_lst))
    logger.info('thread# lst: {}'.format(thread_num_lst))
    logger.info('data set lst: {}'.format(data_set_lst))

    def one_round():
        data_set_name = 'ca-HepTh'
        for our_algorithm in our_exec_name_lst:
            for update_tag in ['ins', 'del']:
                for edge_num in [str(i) for i in range(5000, 30000, 5000)]:
                    statistics_dir = os.sep.join(
                        map(str, [exp_res_root_name, folder_name, data_set_name, update_tag, edge_num]))
                    os.system('mkdir -p ' + os.sep.join([statistics_dir, 'log']))
                    os.system('mkdir -p ' + os.sep.join([statistics_dir, 'dstat']))
                    os.system('mkdir -p ' + os.sep.join([statistics_dir, 'dstat_clean']))
                    statistics_file_path = statistics_dir + os.sep + our_algorithm + '.log'
                    dstat_file_path = os.sep.join([statistics_dir, 'dstat', our_algorithm + '-dstat.log'])
                    log_file_path = os.sep.join([statistics_dir, 'log', our_algorithm + '-raw.log'])
                    logger.info('stat file path: {}'.format(statistics_file_path))

                    # 1st: write header
                    append_header(statistics_file_path)
                    append_header(dstat_file_path)
                    append_header(log_file_path)

                    # 2nd: run exec cmd
                    algorithm_path = our_exec_path + os.sep + our_algorithm
                    params_lst = map(str, ['cgexec -g memory:yche-exp' if with_c_group else '',
                                           algorithm_path + '-' + update_tag, data_set_name, edge_num,
                                           statistics_file_path])
                    cmd = ' '.join(params_lst)
                    logger.info('exec-cmd: {}'.format(cmd))
                    time_out = 800000
                    my_env = os.environ.copy()

                    def execute_cmd(my_cmd):
                        logger.info('sub-process: {}'.format(my_cmd))
                        os.system(my_cmd)

                    # 3rd: spawn a new process to run the exec
                    dstat_cmd = 'dstat -tcdrlmgyn --fs >> ' + dstat_file_path

                    p = Process(target=execute_cmd, args=(dstat_cmd,))
                    p.start()
                    tle_flag, info, correct_info = time_out_util.run_with_timeout(cmd, timeout_sec=time_out,
                                                                                  env=my_env)
                    time_out_util.kill_term_recursive(p.pid)
                    modify_dstat_file(dstat_file_path)

                    # 4th: append outputs
                    write_split(statistics_file_path)
                    with open(statistics_file_path, 'a+') as ifs:
                        ifs.write(correct_info)
                        ifs.write('\nis_time_out:' + str(tle_flag))
                        ifs.write(my_splitter + time.ctime() + my_splitter)
                        ifs.write('\n\n\n\n')
                    if len(info) > 0:
                        with open(log_file_path, 'a+') as ofs:
                            ofs.write(info)

                    logger.info('finish: {}'.format(cmd))

    one_round()


if __name__ == '__main__':
    hostname = socket.gethostname()
    run_exp(env_tag=lccpu12_tag, with_c_group=False)
