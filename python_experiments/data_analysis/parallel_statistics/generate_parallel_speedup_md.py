import decimal

from data_analysis.parallel_statistics.parallel_time_speedup import *

prlp_tag = 'prlp-with-lock'
prlp_lock_free_tag = 'prlp-lock-free'
pflp_tag = 'pflp-with-lock'
rlp_tag = 'rlp'
flp_tag = 'flp'
rlp_old_tag = 'rlp-old'
flp_old_tag = 'flp-old'


def format_str(float_num):
    my_str = str(decimal.Decimal.from_float(float_num).quantize(decimal.Decimal('0.000')))
    return my_str


if __name__ == '__main__':
    with open('../data-json/parallel_exp/scalability_04_24.json') as ifs:
        speedup_data = json.load(ifs)
    with open('../data-json/parallel_exp/seq_time_04_24.json') as ifs:
        seq_time_data = json.load(ifs)
    with open('../data-json/parallel_exp/seq_time_previous.json') as ifs:
        previous_time_data = json.load(ifs)

    parallel_exp_dir = os.sep.join(['..', 'data-markdown', 'parallel'])
    os.system('mkdir -p ' + parallel_exp_dir)

    data_set_lst = [
        'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        'email-Enron', 'email-EuAll',
        'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google', 'cit-Patents',
        'soc-LiveJournal1']


    def get_md_table(algorithm=prlp_tag, tag=speedup_tag, speedup_data=speedup_data):
        lines = []
        for data_set in data_set_lst:
            info_dict = speedup_data[algorithm][data_set]
            if len(lines) == 0:
                lines.append(' | '.join(['dataset'] + map(str, info_dict[thread_lst_tag])))
                lines.append(' | '.join(['---' for _ in range(len(info_dict[thread_lst_tag]) + 1)]))
            lines.append(' | '.join([data_set] + map(format_str, info_dict[tag])))
        return '\n'.join(lines)


    def get_seq_time_table():
        lines = []
        tag_lst = [flp_old_tag, flp_tag, rlp_old_tag, rlp_tag, prlp_tag, prlp_lock_free_tag]
        for data_set in data_set_lst:
            def get_time(tag):
                if tag in [flp_old_tag, rlp_old_tag]:
                    return previous_time_data[tag][data_set]
                return seq_time_data[tag][data_set] if tag in [flp_tag, rlp_tag] else \
                    min(speedup_data[tag][data_set][time_tag])

            if len(lines) == 0:
                lines.append(' | '.join(['dataset'] + tag_lst))
                lines.append(' | '.join(['---' for _ in range(len(tag_lst) + 1)]))
            lines.append(' | '.join([data_set] + map(format_str, map(get_time, tag_lst))))
        return '\n'.join(lines)


    print(get_md_table())
    print(get_seq_time_table())

    # 1st
    with open(os.sep.join([parallel_exp_dir, 'speedup_04_28.md']), 'w') as ofs:
        def write_md_lines(tag):
            ofs.writelines(['\n', '## ' + tag, '\n'])
            ofs.writelines(['\n', '### speedup ', '\n\n', get_md_table(tag, speedup_tag), '\n'])
            ofs.writelines(['\n', '### time ', '\n\n', get_md_table(tag, time_tag), '\n'])


        for tag in [prlp_tag, prlp_lock_free_tag, pflp_tag]:
            write_md_lines(tag)
        ofs.writelines(['\n', '## ' + 'comparison of time', '\n'])
        ofs.writelines(['\n', '### time', '\n\n', get_seq_time_table()])
        # ofs.write_md_lines(['\n', '## pflp ', '\n'])
        # ofs.write_md_lines(['\n', '### speedup ', '\n\n', get_md_table(pflp_tag, speedup_tag), '\n'])
        # ofs.write_md_lines(['\n', '### time ', '\n\n', get_md_table(pflp_tag, time_tag), '\n'])

    # 2nd
    with open('../data-json/parallel_exp/scalability_gen_time_04_24.json') as ifs:
        new_speedup_data = json.load(ifs)
    with open(os.sep.join([parallel_exp_dir, 'gen_time_speedup_04_28.md']), 'w') as ofs:
        def write_md_lines(tag):
            ofs.writelines(['\n', '## ' + tag, '\n'])
            ofs.writelines(['\n', '### speedup ', '\n\n', get_md_table(tag, speedup_tag, new_speedup_data), '\n'])
            ofs.writelines(['\n', '### time ', '\n\n', get_md_table(tag, task_gen_tag, new_speedup_data), '\n'])


        for tag in [prlp_tag, prlp_lock_free_tag]:
            write_md_lines(tag)
