from paper_figures.tkde.data_legacy.static_data_loader import *


def get_algorithm_indexing_time_lst(tag):
    if tag is vldbj_reasd_tag:
        return [0.00522976, 0.00612305, 0.00374263, 0.00103439, 0.00722572, 0.00366369,
                # 0.00847435,
                0.00673668, 0.020272, 0.0117233, 0.00976721, 0.0828088, 0,
                0.00699347, 0.0430287]
    elif tag is vldbj_readrq_tag:
        return [0.000235797, 0.000275629, 0.00020318, 7.15575e-05, 0.000356041, 0.000165915,
                # 0.000449374,
                0.000430675, 0.00149114, 0.000618879, 0.000762481, 0.00199481, 0.0045728,
                0.000455444, 0.00191903]
    # local_push
    else:
        return [0.00130415, 0.00241406, 0.00237629, 0.00163426, 0.0139743, 0.0493718,
                # 0.124753,
                0.102021, 0.271308, 0.268973, 1.25391, 2.47118, 9.09415,
                0.56654, 8.8932]


if __name__ == '__main__':
    logger = get_logger('/home/yche/logs/tmp.log', __name__)
    data_set_lst = [
        "ca-GrQc", "ca-HepTh", "p2p-Gnutella06", "wiki-Vote",
        "email-Enron", "email-EuAll", "digg-friends", "web-Stanford",
        "web-BerkStan", "web-Google", "flickr-growth", "cit-Patents",
        "soc-LiveJournal1", "wiki-Link"
    ]
    with open('inc-sr.json') as ifs:
        inc_sr_dict = json.load(ifs)

    # 1st: time
    time_dict = dict()
    for tag in [vldbj_reasd_tag, vldbj_readrq_tag, vldbj_dlp_tag]:
        time_dict[tag] = dict(zip(data_set_lst, get_algorithm_indexing_time_lst(tag)))
    time_dict[icde_inc_sr_tag] = inc_sr_dict['time']
    time_dict[tkde_pdlp_tag] = dict(
        zip(data_set_lst, [x / 15. for x in get_algorithm_indexing_time_lst(vldbj_dlp_tag)]))
    with open('vldbj-icde-dynamic-time.json', 'w') as ofs:
        ofs.write(json.dumps(time_dict, indent=4))

    with open('vldbj-mem-size.json') as ifs:
        mem_size_dict = json.load(ifs)
        mem_size_dict[icde_inc_sr_tag] = inc_sr_dict['mem']
        mem_size_dict.pop(vldbj_probesim_tag)
        with open('vldbj-icde-dynamic-mem.json', 'w') as ofs:
            ofs.write(json.dumps(mem_size_dict, indent=4))
