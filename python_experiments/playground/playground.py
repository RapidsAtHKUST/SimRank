import json

if __name__ == '__main__':
    my_str = "hello 1112.3 s"
    print(my_str.split('hello')[-1].split('s')[0])
    with open('../data_analysis/data-json/accuracy_result.json') as ifs:
        print(json.load(ifs))
