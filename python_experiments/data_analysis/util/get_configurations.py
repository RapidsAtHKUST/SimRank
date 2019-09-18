from config import *


def get_config_dict_via_hostname(hostname):
    if hostname.startswith('lccpu12'):
        config_tag = lccpu12_tag
    else:
        config_tag = None
    config_dict = get_config_dict(config_tag, '../..')
    return config_dict
