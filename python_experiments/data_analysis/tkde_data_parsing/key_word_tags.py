import json

from data_analysis.util.read_file_utils_updated import json_dir_tag
from exec_utilities.exec_utils import get_logger

max_err_tag = 'Max Error'
avg_err_tag = 'Avg Error'


# Algorithm -> Dataset -> Info-Dictionary (Max Error / Avg Error)
def get_err_by_tag(my_dict, algorithm, data_set, tag):
    assert tag is max_err_tag or tag is avg_err_tag
    return my_dict[algorithm][data_set][tag]


if __name__ == '__main__':
    with open('{}/accuracy_result.json'.format(json_dir_tag)) as ifs:
        logger = get_logger('/home/yche/logs/tmp.log', __name__)
        logger.info(get_err_by_tag(json.load(ifs), 'probesim_ss_ap', 'ca-GrQc', max_err_tag))
