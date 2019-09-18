import os, json

exec_path_tag = 'our_exec_path'
data_set_path_tag = 'data_set_path'

lccpu12_tag = 'lccpu12'

thread_num_tag = 'thread_num'
thread_num_lst_tag = 'thread num lst'
data_set_lst_tag = 'data_set_lst'
gt_data_set_lst_tag = 'gt_data_set_lst'

exp_res_root_path_tag = 'exp_res_root_path'
exp_res_root_mount_path_tag = 'exp_res_root_mount_path'
local_log_tag = 'local_log_path'
hostname_tag = 'hostname'


def get_config_dict(env_tag, config_root='.'):
    with open(config_root + os.sep + 'config.json') as ifs:
        my_config_dict = json.load(ifs)[env_tag]
        return my_config_dict
