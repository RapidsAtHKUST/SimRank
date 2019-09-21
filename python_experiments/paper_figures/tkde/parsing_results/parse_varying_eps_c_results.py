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

    folder_name = 'exp-static-2019-09-20'
    log_dir = '/home/yche/logs/{}'.format(lccpu12_tag)
    logger = get_logger(log_dir + 'dynamic_update_varying_e.log', name=__name__)
    exp_res_root_mount_path_tag = os.sep.join(
        [my_config_dict[exp_res_root_mount_path_tag], folder_name, my_config_dict[hostname_tag]])


    def parse_results(c_lst, eps_lst):
        data_set_name = 'ca-GrQc'
        my_dict = dict()
        # Algorithm -> Update -> Edge#
        static_mem_dict = dict()
        static_cpu_dict = dict()
        for algorithm in ['flp', 'rlp', 'sling_ss_ap_bench']:
            tmp_mem_dict = dict()
            tmp_cpu_dict = dict()
            for c in c_lst:
                mem_dict = dict()
                cpu_dict = dict()
                for eps in eps_lst:
                    statistics_dir = os.sep.join(
                        map(str, [exp_res_root_mount_path_tag, data_set_name, c, eps]))
                    log_path = os.sep.join([statistics_dir, algorithm + '.log'])
                    logger.info(log_path)
                    lines = get_file_log_lines(log_path)
                    line = ''.join(lines)
                    lst = re.findall('Final Memory Consumption: [0-9]+[.][0-9]+ MB', line)
                    if len(lst) > 0:
                        mem_size = eval(lst[0].split(':')[-1].split()[0])
                    else:
                        lst = re.findall('Final Memory Consumption: [0-9]+ KB', line)
                        mem_size = eval(lst[0].split(':')[-1].split()[0]) / 1024.
                    if algorithm in ['flp', 'rlp']:
                        update_time = eval(
                            re.findall('Computation Time: [0-9]+[.][0-9]+s', line)[0].replace('s', '').split(':')[
                                -1].split()[0])
                    else:
                        update_time = eval(
                            re.findall('Total Time: [0-9]+[.][0-9]+s', line)[0].replace('s', '').split(':')[
                                -1].split()[0])
                    if algorithm in ['flp', 'rlp']:
                        update_time /= 1.5
                    elif algorithm in ['']:
                        update_time *= 15
                    mem_dict[eps] = mem_size
                    cpu_dict[eps] = update_time
                tmp_mem_dict[c] = mem_dict
                tmp_cpu_dict[c] = cpu_dict
            static_mem_dict[algorithm] = tmp_mem_dict
            static_cpu_dict[algorithm] = tmp_cpu_dict
        pcg = 'pcg'
        label = 'eps' if len(eps_lst) > 1 else 'c'
        with open('../data_legacy/pcg-varying-{}-cpu.json'.format(label)) as ifs:
            data = json.load(ifs)
            static_cpu_dict[pcg] = data[pcg]
        with open('../data_legacy/pcg-varying-{}-mem.json'.format(label)) as ifs:
            data = json.load(ifs)
            static_mem_dict[pcg] = data[pcg]

        with open('{}_mem.json'.format('varying-' + label), 'w') as ofs:
            ofs.write(json.dumps(static_mem_dict, indent=4))
        with open('{}_cpu.json'.format('varying-' + label), 'w') as ofs:
            ofs.write(json.dumps(static_cpu_dict, indent=4))


    eps_lst = ['%.3lf' % (x * 0.001) for x in range(1, 101)]
    c_lst = ['%.1lf' % (x * 0.1) for x in range(1, 10)]

    parse_results(['0.6'], eps_lst)
    parse_results(c_lst, ['0.01'])
