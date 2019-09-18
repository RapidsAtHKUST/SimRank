from data_analysis.generate_index_markdown import get_index_dict, sling_tag, data_set_lst, format_str
from data_analysis.generate_speedup_over_sling_markdown import lst_divide

our_avg_index_update_lst = [0.00130415, 0.00241406, 0.00237629, 0.00163426, 0.0139743, 0.0493718,
                            # 0.124753,
                            0.102021, 0.271308, 0.268973, 1.25391, 2.47118]

sling_lst = map(lambda data_set: get_index_dict()[sling_tag]['indexing time'][data_set], data_set_lst)

speedup_lst = lst_divide(sling_lst, our_avg_index_update_lst)


def get_speedup_table(start, end):
    # make the table
    table_lines = []
    header = ['algo\\data'] + data_set_lst[start:end]
    table_lines.append(' | '.join(header))
    table_lines.append(' | '.join(['---'] * (len(data_set_lst[start:end]) + 1)))
    lines = map(lambda algorithm:
                ' | '.join(
                    ['flp speedup over ' + algorithm] +
                    map(lambda num: format_str(num),
                        speedup_lst[start:end], ))
                , [sling_tag])
    table_lines.extend(lines)
    return '\n'.join(table_lines)


if __name__ == '__main__':
    with open('data-markdown/index_update_speedup_over_sling.md', 'w') as ofs:
        ofs.write('\n'.join([get_speedup_table(0, 6), '\n', get_speedup_table(6, 11), '\n']))
