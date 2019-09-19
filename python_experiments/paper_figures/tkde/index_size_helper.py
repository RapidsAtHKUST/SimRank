import json

if __name__ == '__main__':
    with open('data-legacy/index-size.dict') as ifs:
        index_info = eval(ifs.readline())
        with open('data-legacy/index.json', 'w') as ofs:
            ofs.write(json.dumps(index_info, indent=4))
