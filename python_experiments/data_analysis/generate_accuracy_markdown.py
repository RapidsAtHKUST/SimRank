from querying_time_accuracy_statistics import *
import decimal


def get_accuracy_dict(root_dir='.'):
    with open(root_dir + os.sep + 'data-json' + os.sep + 'accuracy_result' + '.json') as ifs:
        return json.load(ifs)


def format_str(float_num):
    return str(decimal.Decimal.from_float(float_num * (10 ** 3)).quantize(decimal.Decimal('0.000')))


if __name__ == '__main__':
    algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag, isp_tag, tsf_tag, lind_tag, cw_tag]

    accuracy_dict = get_accuracy_dict()


    def get_time_table(round_lst, data_set):
        table_lines = []
        header = ['algo\\data'] + map(str, round_lst)
        table_lines.append(' | '.join(header))
        table_lines.append(' | '.join(['---'] * (len(round_lst) + 1)))
        lines = map(lambda algorithm:
                    ' | '.join([algorithm] +
                               map(lambda num: format_str(num) + " e-3",
                                   accuracy_dict[algorithm][data_set][str(10 ** 5)], )), algorithm_tag_lst)
        table_lines.extend(lines)
        return '\n'.join(table_lines)


    os.system('mkdir -p data-markdown')
    with open('data-markdown/accuracy_result.md', 'w') as ofs:
        for data_set in accuracy_data_set_lst:
            ofs.writelines(['## ' + data_set, '\n\n', get_time_table(range(10), data_set), '\n\n'])
