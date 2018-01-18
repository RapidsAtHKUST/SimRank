from indexing_time_space_statistics import *


def get_index_dict(root_dir='.'):
    with open(root_dir + os.sep + 'data-json' + os.sep + 'index_result' + '.json') as ifs:
        lines = ifs.readlines()
        return eval(''.join(lines))


if __name__ == '__main__':
    algorithm_lst = [local_push_tag, sling_tag, linear_d_tag, cloud_walker_tag, tsf_tag]

    index_dict = get_index_dict()


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

        ofs.writelines(['## Max Mem Consumption', '\n\n'])
        ofs.writelines([get_mem_size_table(data_set_lst[0:6]), '\n\n'])
        ofs.writelines(
            [get_mem_size_table(data_set_lst[6:len(data_set_lst)]), '\n\n'])
