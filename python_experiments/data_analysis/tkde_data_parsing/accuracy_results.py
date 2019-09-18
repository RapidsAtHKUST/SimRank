from config import *
from data_analysis.util.read_file_utils_updated import *
from data_analysis.util.parsing_helpers import *
from exec_utilities.exec_utils import get_logger
from data_analysis.tkde_data_parsing.key_word_tags import *
import json

if __name__ == '__main__':
    env_tag = lccpu12_tag
    with open('../../config.json') as ifs:
        my_config_dict = json.load(ifs)[env_tag]
    gt_data_set_lst = my_config_dict[gt_data_set_lst_tag]
    thread_num_lst = my_config_dict[thread_num_lst_tag]

    folder_name = 'exp-ground-truth-2019-09-18'
    our_exec_name_lst = ['probesim_ss_ap', 'readsrq-ap', 'readsd-ap', 'sling_ss_ap']

    log_dir = '/home/yche/logs/{}'.format(lccpu12_tag)
    logger = get_logger(log_dir + 'accuracy_results.log', name=__name__)
    exp_res_root_mount_path_tag = os.sep.join(
        [my_config_dict[exp_res_root_mount_path_tag], folder_name, my_config_dict[hostname_tag]])

    os.system('mkdir -p {}'.format(json_dir_tag))
    os.system('mkdir -p {}'.format(md_dir_tag))

    json_dict = dict()
    # Algorithm -> Dataset -> Info-Dictionary (Max Error / Avg Error)
    for our_algorithm in our_exec_name_lst:
        my_dict = dict()
        for data_set_name in gt_data_set_lst:
            t_num = thread_num_lst[0]
            statistics_dir = os.sep.join(
                map(str, [exp_res_root_mount_path_tag, data_set_name, t_num]))
            log_path = os.sep.join([statistics_dir, our_algorithm + '.log'])
            logger.info(log_path)
            lines = get_file_log_lines(log_path)

            info = dict()
            for tag in [max_err_tag, avg_err_tag]:
                res_lst = parse_line_split(list(filter(lambda x: contains([max_err_tag], x), lines))[0])
                info[tag] = None if len(res_lst) == 0 else res_lst[-1]
            # logger.info(my_dict)
            my_dict[data_set_name] = info
        json_dict[our_algorithm] = my_dict
    logger.info(json_dict)
    with open('{}/accuracy_result.json'.format(json_dir_tag), 'w') as ofs:
        ofs.write(json.dumps(json_dict, indent=4))
