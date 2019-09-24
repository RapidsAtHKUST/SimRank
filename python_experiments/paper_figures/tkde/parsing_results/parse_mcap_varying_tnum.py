from config import *
from data_analysis.util.read_file_utils_updated import *
from data_analysis.tkde_data_parsing.key_word_tags import *
import json, re


def bak():
    inc_sr_time_ins = {5000: 828.2532715, 10000: 1583.839004, 15000: 2357.798905, 20000: 3135.9563235,
                       25000: 3902.367272}
    inc_sr_mem_ins = {5000: 1207.69154, 10000: 1209.622656, 15000: 1222.702304, 20000: 1234.045476, 25000: 1236.55864}

    inc_sr_time = {5000: 780.4734544, 10000: 1545.050671, 15000: 2355.302817428571, 20000: 3053.92380275,
                   25000: 3873.9596019444443}
    inc_sr_mem = {5000: 1236.55864, 10000: 1252.076852, 15000: 1264.300012, 20000: 1268.899444, 25000: 1271.284524}
    return inc_sr_time_ins, inc_sr_mem_ins, inc_sr_time, inc_sr_mem


if __name__ == '__main__':
    env_tag = lccpu12_tag
    with open('../../../config.json') as ifs:
        my_config_dict = json.load(ifs)[env_tag]
    gt_data_set_lst = my_config_dict[gt_data_set_lst_tag] + my_config_dict[data_set_lst_tag]
    thread_num_lst = my_config_dict[thread_num_lst_tag]

    folder_name = 'exp-lind-scalability-2019-09-23'
    log_dir = '/home/yche/logs/{}'.format(lccpu12_tag)
    logger = get_logger(log_dir + 'dynamic_update_varying_e.log', name=__name__)
    exp_res_root_mount_path_tag = os.sep.join(
        [my_config_dict[exp_res_root_mount_path_tag], folder_name, my_config_dict[hostname_tag]])


    def parse_results():
        # Algorithm -> Dataset -> Thread #
        for algorithm in [
            # 'LinSimBench',
            'CloudWalkerBench']:
            for data_set in ['web-Stanford', 'web-Google', 'cit-Patents', 'soc-LiveJournal1', ]:
                for t_num in ['1', '2', '4', '8', '16', '32', '56', '64']:
                    statistics_dir = os.sep.join(
                        map(str, [exp_res_root_mount_path_tag, data_set, t_num]))
                    log_path = os.sep.join([statistics_dir, algorithm + '.log'])
                    logger.info(log_path)
                    if not os.path.exists(log_path):
                        update_time = 0
                    else:
                        lines = get_file_log_lines(log_path)
                        line = ''.join(lines)
                        if len(re.findall('query time:', line)) == 0:
                            update_time = 0
                        else:
                            update_time = eval(
                                re.findall('query time: [0-9]+[.][0-9]+s', line)[0].replace('s', '').split(':')[
                                    -1].split()[0])
                    print(update_time)


    parse_results()
