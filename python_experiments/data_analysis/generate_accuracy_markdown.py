from querying_time_accuracy_statistics import *
from reads_accuracy_statistics import *
import decimal


def get_accuracy_dict(root_dir='.', file_name='accuracy_result'):
    with open(root_dir + os.sep + 'data-json' + os.sep + file_name + '.json') as ifs:
        return json.load(ifs)


def get_accuracy_dict_with_reads(root_dir='.', file_name='accuracy_result'):
    accuracy_dict = get_accuracy_dict(root_dir)

    for file in ['accuracy_result_reads', 'accuracy_result_probesim']:
        read_dict = get_accuracy_dict(root_dir, file_name=file)
        assert isinstance(read_dict, dict)
        for key, val in read_dict.iteritems():
            accuracy_dict[key] = val
    return accuracy_dict


def format_str(float_num):
    my_str = str(decimal.Decimal.from_float(float_num * (10 ** 2)).quantize(decimal.Decimal('0.000')))
    if (float_num < 0.01):
        return my_str
    else:
        return '**' + my_str + '**'


if __name__ == '__main__':
    algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag,
                         reads_tag, reads_d_tag, reads_rq_tag,
                         isp_tag, tsf_tag, lind_tag, cw_tag]

    accuracy_dict = get_accuracy_dict_with_reads()


    def get_time_table(round_lst, data_set):
        table_lines = []
        header = ['algo\\data'] + map(str, round_lst)
        table_lines.append(' | '.join(header))
        table_lines.append(' | '.join(['---'] * (len(round_lst) + 1)))
        lines = map(lambda algorithm:
                    ' | '.join([algorithm] +
                               map(lambda num: format_str(num) + "",
                                   accuracy_dict[algorithm][data_set][str(10 ** 5)], )), algorithm_tag_lst)
        table_lines.extend(lines)
        return '\n'.join(table_lines)


    os.system('mkdir -p data-markdown')
    with open('data-markdown/accuracy_result.md', 'w') as ofs:
        ofs.writelines(['# Max Err, Unit: 0.01\n\n'])
        for data_set in accuracy_data_set_lst:
            ofs.writelines(['## ' + data_set, '\n\n', get_time_table(range(10), data_set), '\n\n'])
