import json
import os
from data_analysis.parallel_statistics.generate_parallel_speedup_md import format_str

rlp_old_tag = 'rlp-old'
flp_old_tag = 'flp-old'
pcg_tag = 'pcg'
cw_tag = 'mcap'
lin_tag = 'lind'

if __name__ == '__main__':
    data_set_lst = [
        'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        'email-Enron', 'email-EuAll', 'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google',
        'cit-Patents', 'soc-LiveJournal1'
    ]
    rlp_time = [1.07741, 1.74568, 1.47582, 2.19363, 47.5605, 18.0652, 1797.56, 917.322, 2120.15, 123.402, 919.906,
                20132.5]
    rlp_mem = [19256.0, 51528.0, 73596.0, 40416.0, 727268.0, 682672.0, 44717996.0, 3680064.0, 7523072.0, 3006152.0,
               27553756.0, 171345120.0]

    flp_time = [1.85332, 3.45821, 2.8585, 3.8606, 88.9838, 33.9145, 4063.06, 1751.85, 4403.81, 234.058, 1707.68,
                31980.9]
    flp_mem = [30872.0, 91232.0, 140344.0, 64960.0, 1418748.0, 1203860.0, 89870480.0, 6963008.0, 13929776.0, 5088512.0,
               51468080.0, 334798996.0]

    pcg_time = [6.88264, 25.0051, 20.8719, 10.1768, 648.495, 0, 0, 0, 0, 0, 0, 0]
    pcg_mem = [756888.0, 2675850.0, 2084280.0, 1397940.0, 36857500.0, 0, 0, 0, 0, 0, 0, 0]

    lin_time = [12.625995767059994, 42.02774043701001, 29.68137939426999, 27.032589475949997, 667.9754282328001,
                20304.824625019006, 45336.83949782399, 50572.94144942692, 327914.2239266596, 564967.8041761469,
                12274249.223928338, 39868660.06281909]
    lin_mem = [7687.744, 11407.724, 8679.656, 16931.932, 56399.988, 118223.736, 251975.888, 351043.688, 1027347.832,
               769061.924, 2637583.876, 8935494.748]

    cw_time = [255.23707029200023, 548.1131236445002, 454.8569550818002, 312.6183089454998, 3417.5582213816,
               63617.12814739399, 88902.4341619849, 72311.10883203904, 383753.02945318044, 599341.561741178,
               11261417.728323543, 19675654.326498616]
    cw_mem = [37007.988, 10495.88, 8211.984, 18027.988, 50211.728, 104287.892, 230895.768, 300315.9, 925849.188,
              716370.592, 8053419.368, 119997691.736]

    tag_lst = [rlp_old_tag, flp_old_tag, pcg_tag, cw_tag, lin_tag]
    time_lst_dict = {
        rlp_old_tag: rlp_time,
        flp_old_tag: flp_time,
        pcg_tag: pcg_time,
        lin_tag: lin_time,
        cw_tag: cw_time
    }
    mem_lst_dict = {
        rlp_old_tag: rlp_mem,
        flp_old_tag: flp_mem,
        pcg_tag: pcg_mem,
        lin_tag: lin_mem,
        cw_tag: cw_mem
    }

    time_stat_dict = {}
    for tag in tag_lst:
        time_stat_dict[tag] = dict(zip(data_set_lst, map(float,map(format_str, time_lst_dict[tag]))))
    mem_stat_dict = {}
    for tag in tag_lst:
        mem_stat_dict[tag] = dict(zip(data_set_lst, mem_lst_dict[tag]))

    root_dir_path = '../data-json/parallel_exp'
    os.system('mkdir -p ' + root_dir_path)
    with open(os.sep.join([root_dir_path, 'seq_time_previous.json']), 'w') as ofs:
        ofs.write(json.dumps(time_stat_dict, indent=4))
    with open(os.sep.join([root_dir_path, 'seq_mem_previous.json']), 'w') as ofs:
        ofs.write(json.dumps(mem_stat_dict, indent=4))
