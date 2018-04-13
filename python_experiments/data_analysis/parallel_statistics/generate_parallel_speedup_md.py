import decimal

from parallel_time_speedup import *

prlp_tag = 'prlp'
pflp_tag = 'pflp'


def format_str(float_num):
    my_str = str(decimal.Decimal.from_float(float_num).quantize(decimal.Decimal('0.000')))
    return my_str


if __name__ == '__main__':
    with open('../data-json/parallel_exp/scalability_04_12.json') as ifs:
        speedup_data = json.load(ifs)

    parallel_exp_dir = os.sep.join(['..', 'data-markdown', 'parallel'])
    os.system('mkdir -p ' + parallel_exp_dir)

    data_set_lst = [
        'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        'email-Enron', 'email-EuAll',
        'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google', 'cit-Patents',
        'soc-LiveJournal1']


    def get_md_table(algorithm=prlp_tag, tag=speedup_tag):
        lines = []
        for data_set in data_set_lst:
            info_dict = speedup_data[algorithm][data_set]
            if len(lines) == 0:
                lines.append(' | '.join(['dataset'] + map(str, info_dict[thread_lst_tag])))
                lines.append(' | '.join(['---' for _ in xrange(len(info_dict[thread_lst_tag]) + 1)]))
            lines.append(' | '.join([data_set] + map(format_str, info_dict[tag])))
        return '\n'.join(lines)


    print get_md_table()

    with open(os.sep.join([parallel_exp_dir, 'speedup_04_12.md']), 'w') as ofs:
        ofs.writelines(['\n', '## prlp ', '\n'])
        ofs.writelines(['\n', '### speedup ', '\n\n', get_md_table(prlp_tag, speedup_tag), '\n'])
        ofs.writelines(['\n', '### time ', '\n\n', get_md_table(prlp_tag, time_tag), '\n'])

        # ofs.writelines(['\n', '## pflp ', '\n'])
        # ofs.writelines(['\n', '### speedup ', '\n\n', get_md_table(pflp_tag, speedup_tag), '\n'])
        # ofs.writelines(['\n', '### time ', '\n\n', get_md_table(pflp_tag, time_tag), '\n'])
