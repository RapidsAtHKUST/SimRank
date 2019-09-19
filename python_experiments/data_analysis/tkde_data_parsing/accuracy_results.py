from config import *
from data_analysis.util.read_file_utils_updated import *
from data_analysis.util.parsing_helpers import *
from data_analysis.tkde_data_parsing.key_word_tags import *
import json

if __name__ == '__main__':
    env_tag = lccpu12_tag
    with open('../../config.json') as ifs:
        my_config_dict = json.load(ifs)[env_tag]
    gt_data_set_lst = my_config_dict[gt_data_set_lst_tag]
    thread_num_lst = my_config_dict[thread_num_lst_tag]

    folder_name = 'exp-ground-truth-2019-09-18'
    log_dir = '/home/yche/logs/{}'.format(lccpu12_tag)
    logger = get_logger(log_dir + 'accuracy_results.log', name=__name__)
    exp_res_root_mount_path_tag = os.sep.join(
        [my_config_dict[exp_res_root_mount_path_tag], folder_name, my_config_dict[hostname_tag]])

    os.system('mkdir -p {}'.format(json_dir_tag))
    os.system('mkdir -p {}'.format(md_dir_tag))


    def parse_results():
        json_dict = dict()
        # Algorithm -> Dataset -> Info-Dictionary (Max Error / Avg Error)
        for our_algorithm in accuracy_exec_lst:
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
                    res_lst = parse_line_split(list(filter(lambda x: contains([tag], x), lines))[0])
                    info[tag] = None if len(res_lst) == 0 else '{0:.5e}'.format(float(res_lst[-1]))
                # logger.info(my_dict)
                my_dict[data_set_name] = info
            json_dict[our_algorithm] = my_dict
        logger.info(json_dict)
        with open('{}/accuracy_result.json'.format(json_dir_tag), 'w') as ofs:
            ofs.write(json.dumps(json_dict, indent=4))


    # parse_results()
    def generate_latex_table():
        with open('{}/accuracy_result.json'.format(json_dir_tag)) as ifs:
            data_dict = json.load(ifs)
            for our_algorithm in [sling_ss_ap, probesim_aps_tag, readsd_aps_tag, readsrq_aps_tag]:
                lst = [alias_dict[our_algorithm]]
                for data_set in gt_data_set_lst:
                    for tag in [max_err_tag, avg_err_tag]:
                        lst.append(get_err_by_tag(data_dict, our_algorithm, data_set, tag))
                logger.info(' & '.join([str(x) for x in lst]) + ' \\\\ \hline')


    parse_results()
    generate_latex_table()
