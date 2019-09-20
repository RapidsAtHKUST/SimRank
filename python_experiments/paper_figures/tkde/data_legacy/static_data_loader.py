import json

from exec_utilities.exec_utils import get_logger

vldbj_sling_tag = 'sling'
vldbj_reasd_tag = 'reads-d'
vldbj_readrq_tag = 'reads-rq'
vldbj_probesim_tag = 'ProbeSim'

vldbj_indexing_time_tag = 'indexing time'
vldbj_index_size_tag = 'index disk size'

size_g = {
    "ca-GrQc": (5242, 14496),
    "p2p-Gnutella06": (8717, 31525),
    "ca-HepTh": (9877, 25998),
    "wiki-Vote": (7115, 103689),
    "web-NotreDame": (325729, 1497134),
    "web-Stanford": (281903, 2312497),
    "web-BerkStan": (685230, 7600595),
    "web-Google": (875713, 5105039),
    "cit-Patents": (3774768, 16518948),
    "soc-LiveJournal1": (4847571, 68993773),
    "email-Enron": (36692, 183831),
    "email-EuAll": (265214, 420045)
}


def get_legacy_data(relative_path: str = '.'):
    with open('{}/probesim.json'.format(relative_path)) as ifs:
        probesim_mem_size = json.load(ifs)

    with open('{}/query.json'.format(relative_path)) as ifs:
        query = json.load(ifs)

    with open('{}/index.json'.format(relative_path)) as ifs:
        index = json.load(ifs)
    return query, index, probesim_mem_size


def get_computation_time(dataset, tag, query, index):
    query_time = query[tag][dataset] * size_g[dataset][0]
    sling_parallel_ap_time_dict = {
        'ca-GrQc': 0.272663665,
        'ca-HepTh': 0.566204357,
        'p2p-Gnutella06': 0.566204357,
        'wiki-Vote': 0.115870139,
        'email-Enron': 18.440552859,
        'email-EuAll': 142.367051626,
        'web-Stanford': 4220.202582435,
        'web-BerkStan': 23623.203228326,
        'web-Google': 27734.27,
    }
    if tag is vldbj_sling_tag:
        # query_time *= 10  # single-pair vs single-source
        if dataset in sling_parallel_ap_time_dict:
            query_time = sling_parallel_ap_time_dict[dataset] * 15  # parallel efficiency : 15
        else:
            query_time *= 100
    if tag is vldbj_probesim_tag:
        return query_time
    else:
        indexing_time = index[tag][vldbj_indexing_time_tag][dataset]
        return query_time + indexing_time


def get_mem_size(dataset, tag, index, probesim_mem_size):
    csr_mem_size = probesim_mem_size[dataset]
    if tag in [vldbj_sling_tag, vldbj_reasd_tag, vldbj_readrq_tag]:
        csr_mem_size += index[tag][vldbj_index_size_tag][dataset]
    return csr_mem_size


vldbj_comp_time_json_file = 'vldbj-comp.json'
vldbj_mem_size_json_file = 'vldbj-mem-size.json'

if __name__ == '__main__':
    logger = get_logger('/home/yche/logs/tmp.log', __name__)
    query, index, probesim_mem_size = get_legacy_data()
    with open('tkde_data_set_lst.json') as ifs:
        data_set_lst = json.load(ifs)
    computation_time_dict = dict()
    for tag in [vldbj_sling_tag, vldbj_probesim_tag, vldbj_reasd_tag, vldbj_readrq_tag]:
        time_dict = dict()
        for data in data_set_lst:
            time = get_computation_time(data, tag, query, index)
            time_dict[data] = time
            logger.info('{} {} : {} seconds', tag, data, time)
        computation_time_dict[tag] = time_dict
    with open(vldbj_comp_time_json_file, 'w') as ofs:
        ofs.write(json.dumps(computation_time_dict, indent=4))

    mem_size_dict = dict()
    for tag in [vldbj_sling_tag, vldbj_probesim_tag, vldbj_reasd_tag, vldbj_readrq_tag]:
        mem_dict = dict()
        for data in data_set_lst:
            mem_size = get_mem_size(data, tag, index, probesim_mem_size)
            mem_dict[data] = mem_size
            logger.info('{} {} : {} MB'.format(tag, data, mem_size))
        mem_size_dict[tag] = mem_dict
    with open(vldbj_mem_size_json_file, 'w') as ofs:
        ofs.write(json.dumps(mem_size_dict, indent=4))
