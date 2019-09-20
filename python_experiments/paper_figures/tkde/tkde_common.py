import json
import os


def get_name_dict():
    with open('data_names.json') as ifs:
        return eval(''.join(ifs.readlines()))


# data set abbreviation dictionary
data_names = get_name_dict()

# figure parameters
FIG_SIZE_MULTIPLE = (32, 4)
LABEL_SIZE = 22
TICK_SIZE = 22
LEGEND_SIZE = 22

os.system('mkdir -p {}'.format('figures'))

with open('tkde_data_set_lst.json') as ifs:
    data_set_lst = json.load(ifs)
g_names = list(map(lambda data: data_names[data], data_set_lst))
size_of_fig = (FIG_SIZE_MULTIPLE)
