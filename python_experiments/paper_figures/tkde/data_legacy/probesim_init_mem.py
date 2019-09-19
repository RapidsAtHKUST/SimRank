from config import *
from data_analysis.util.read_file_utils_updated import *
from data_analysis.tkde_data_parsing.key_word_tags import *
import json, re

if __name__ == '__main__':
    env_tag = lccpu12_tag
    with open('../../../config.json') as ifs:
        my_config_dict = json.load(ifs)[env_tag]
    gt_data_set_lst = my_config_dict[gt_data_set_lst_tag] + my_config_dict[data_set_lst_tag]
    thread_num_lst = my_config_dict[thread_num_lst_tag]

    folder_name = 'exp-ground-truth-2019-09-18'
    log_dir = '/home/yche/logs/{}'.format(lccpu12_tag)
    logger = get_logger(log_dir + 'probesim_mem.log', name=__name__)
    exp_res_root_mount_path_tag = os.sep.join(
        [my_config_dict[exp_res_root_mount_path_tag], folder_name, my_config_dict[hostname_tag]])


    def parse_results():
        # Algorithm -> Dataset -> Info-Dictionary (Max Error / Avg Error)
        our_algorithm = probesim_aps_tag
        my_dict = dict()
        for data_set_name in gt_data_set_lst:
            t_num = thread_num_lst[0]
            statistics_dir = os.sep.join(
                map(str, [exp_res_root_mount_path_tag, data_set_name, t_num, 'log']))
            log_path = os.sep.join([statistics_dir, our_algorithm + '-raw.log'])
            logger.info(log_path)
            lines = get_file_log_lines(log_path)
            line = ''.join(lines)
            mem_size = eval(
                re.findall('Initial Memory Consumption: [0-9]+ KB', line)[0].split(':')[-1].split()[0]) / 1024.
            my_dict[data_set_name] = mem_size
        logger.info(my_dict)
        with open('{}/probesim-mem.json'.format('.'), 'w') as ofs:
            ofs.write(json.dumps(my_dict, indent=4))


    parse_results()
