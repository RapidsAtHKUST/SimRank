import json

from paper_figures.tkde.data_legacy.static_data_loader import vldbj_reasd_tag, vldbj_readrq_tag, vldbj_probesim_tag


def get_reads_probesim_ap_time_dict(tag, data_set_lst):
    if tag in [vldbj_reasd_tag, vldbj_readrq_tag, vldbj_probesim_tag]:
        with open('data_legacy/vldbj-comp.json') as ifs:
            tmp = json.load(ifs)[tag]
            return dict(zip(data_set_lst, [tmp[data_set] for data_set in data_set_lst]))
