from generate_index_markdown import *
from generate_speedup_over_sling_markdown import lst_divide


def time_lst_per_algorithm(algorithm, data_set_lst):
    return map(lambda data_set: index_dict[algorithm][index_time_tag][data_set], data_set_lst)


def get_algorithm_time_lst(data_set_lst):
    return map(lambda algorithm:
               map(lambda data_set: index_dict[algorithm][index_time_tag][data_set], data_set_lst), algorithm_lst)


def get_time_table(data_set_lst):
    table_lines = []
    header = ['algo\\data'] + data_set_lst
    table_lines.append(' | '.join(header))
    table_lines.append(' | '.join(['---'] * (len(data_set_lst) + 1)))
    lines = map(lambda algorithm:
                ' | '.join(
                    [algorithm] +
                    map(lambda num: format_str(num) + " s",
                        map(lambda data_set: index_dict[algorithm][index_time_tag][data_set], data_set_lst)))
                , algorithm_lst)
    table_lines.extend(lines)
    return '\n'.join(table_lines)


def get_speedup_table(data_set_lst):
    algorithm_index_time_lst = get_algorithm_time_lst(data_set_lst)
    flp_time_lst = time_lst_per_algorithm(local_push_tag, data_set_lst)
    speedup_lst = map(lambda time_lst: lst_divide(time_lst, flp_time_lst), algorithm_index_time_lst)
    speedup_lst = dict(zip(algorithm_lst, speedup_lst))
    print speedup_lst

    # make the table
    table_lines = []
    header = ['algo\\data'] + data_set_lst
    table_lines.append(' | '.join(header))
    table_lines.append(' | '.join(['---'] * (len(data_set_lst) + 1)))
    lines = map(lambda algorithm: ' | '.join(
        [algorithm] + map(lambda num: format_str(num), speedup_lst[algorithm], )), algorithm_lst)
    table_lines.extend(lines)
    return '\n'.join(table_lines)


if __name__ == '__main__':
    algorithm_lst = [
        # reads_tag,
        reads_d_tag, reads_rq_tag, sling_tag, tsf_tag, linear_d_tag, cloud_walker_tag]
    index_dict = get_index_dict_with_reads()

    print get_time_table(data_set_lst)

    with open('data-markdown/indexing_time_speedup.md', 'w') as ofs:
        ofs.writelines([get_speedup_table(data_set_lst[0:6]), '\n\n'])
        ofs.writelines([get_speedup_table(data_set_lst[6:13]), '\n\n'])
