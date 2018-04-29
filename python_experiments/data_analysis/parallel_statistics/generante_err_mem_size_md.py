from generate_parallel_speedup_md import format_str
from err_mem_size_statistics import *

if __name__ == '__main__':
    data_set_lst = [
        'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        'email-Enron', 'email-EuAll',
        'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google', 'cit-Patents',
        'soc-LiveJournal1']
    small_data_set = data_set_lst[0:4]

    with open('../data-json/parallel_exp/err_mem_size04_24.json') as ifs:
        err_mem_size_dict = json.load(ifs)


    def get_seq_time_table(info_tag):
        lines = []
        tag_lst = [flp_tag, rlp_tag, prlp_tag, prlp_lock_free_tag]
        data_set_iter = data_set_lst if info_tag == mem_size_tag else small_data_set
        for data_set in data_set_iter:
            if len(lines) == 0:
                lines.append(' | '.join(['dataset'] + tag_lst))
                lines.append(' | '.join(['---' for _ in xrange(len(tag_lst) + 1)]))
            if info_tag == mem_size_tag:
                lines.append(
                    ' | '.join([data_set] + map(lambda tag: format_str(
                        float(err_mem_size_dict[tag][data_set][info_tag]) / 1024), tag_lst)))
            else:
                lines.append(
                    ' | '.join([data_set] + map(lambda tag: '{:.5e}'.format(float(
                        err_mem_size_dict[tag][data_set][info_tag])), tag_lst)))

        return '\n'.join(lines)


    print get_seq_time_table(mean_err_tag), '\n'
    print get_seq_time_table(max_err_tag), '\n'
    print get_seq_time_table(mem_size_tag)

    parallel_exp_dir = os.sep.join(['..', 'data-markdown', 'parallel'])
    with open(os.sep.join([parallel_exp_dir, 'err_mem_size_04_28.md']), 'w') as ofs:
        def write_md_lines(tag):
            ofs.writelines(['\n', '## ' + tag, '\n'])
            ofs.writelines(['\n', get_seq_time_table(tag), '\n'])


        for tag in [mean_err_tag, max_err_tag, mem_size_tag]:
            write_md_lines(tag)
        # ofs.write_md_lines(['\n', '## pflp ', '\n'])
        # ofs.write_md_lines(['\n', '### speedup ', '\n\n', get_md_table(pflp_tag, speedup_tag), '\n'])
        # ofs.write_md_lines(['\n', '### time ', '\n\n', get_md_table(pflp_tag, time_tag), '\n'])
