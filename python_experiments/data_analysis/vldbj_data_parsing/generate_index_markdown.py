from data_analysis.vldbj_data_parsing.indexing_time_space_statistics import *
from data_analysis.vldbj_data_parsing.reads_indexing_statistics import reads_rq_tag, reads_d_tag


def get_index_dict(root_dir='.', file_name='index_result'):
    with open(root_dir + os.sep + 'data-json' + os.sep + file_name + '.json') as ifs:
        return json.load(ifs)


def get_index_dict_with_reads(root_dir='.'):
    index_dict = get_index_dict(root_dir)
    read_dict = get_index_dict(root_dir, file_name='index_result_reads')
    assert isinstance(read_dict, dict)
    for key, val in read_dict.items():
        index_dict[key] = val
    return index_dict


if __name__ == '__main__':
    algorithm_lst = [local_push_tag,
                     # reads_tag,
                     reads_d_tag, reads_rq_tag, sling_tag,
                     linear_d_tag, cloud_walker_tag, tsf_tag]

    index_dict = get_index_dict_with_reads()


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


    def get_index_size_table(data_set_lst):
        table_lines = []
        header = ['algo\\data'] + data_set_lst
        table_lines.append(' | '.join(header))
        table_lines.append(' | '.join(['---'] * (len(data_set_lst) + 1)))
        lines = map(lambda algorithm:
                    ' | '.join(
                        [algorithm] +
                        map(lambda num: format_str(num) + " MB",
                            map(lambda data_set: index_dict[algorithm][index_size_tag][data_set], data_set_lst)))
                    , algorithm_lst)
        table_lines.extend(lines)
        return '\n'.join(table_lines)


    def get_mem_size_table(data_set_lst):
        table_lines = []
        header = ['algo\\data'] + data_set_lst
        table_lines.append(' | '.join(header))
        table_lines.append(' | '.join(['---'] * (len(data_set_lst) + 1)))
        lines = map(lambda algorithm:
                    ' | '.join(
                        [algorithm] +
                        map(lambda num: format_str(num) + " MB",
                            map(lambda data_set: index_dict[algorithm][max_mem_size_tag][data_set], data_set_lst)))
                    , algorithm_lst)
        table_lines.extend(lines)
        return '\n'.join(table_lines)


    os.system('mkdir -p data-markdown')
    with open('data-markdown/index_result.md', 'w') as ofs:
        ofs.writelines(['## Indexing Time', '\n\n'])
        ofs.writelines([get_time_table(data_set_lst[0:6]), '\n\n'])
        ofs.writelines([get_time_table(data_set_lst[6:len(data_set_lst)]), '\n\n'])

        ofs.writelines(['## Index Size', '\n\n'])
        ofs.writelines([get_index_size_table(data_set_lst[0:6]), '\n\n'])
        ofs.writelines([get_index_size_table(data_set_lst[6:len(data_set_lst)]), '\n\n'])

        # ofs.write_md_lines(['## Max Mem Consumption', '\n\n'])
        # ofs.write_md_lines([get_mem_size_table(data_set_lst[0:6]), '\n\n'])
        # ofs.write_md_lines(
        #     [get_mem_size_table(data_set_lst[6:len(data_set_lst)]), '\n\n'])
