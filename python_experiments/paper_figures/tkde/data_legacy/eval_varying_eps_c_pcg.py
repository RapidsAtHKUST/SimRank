import json

if __name__ == '__main__':
    with open('varying_eps_c.dicts') as ifs:
        inc_sr_varying_eps_cpu = eval(ifs.readline())
        inc_sr_varying_eps_mem = eval(ifs.readline())

        inc_sr_varying_c_cpu = eval(ifs.readline())
        inc_sr_varying_c_mem = eval(ifs.readline())

        inc_sr_tag = 'Inc-SR'

        with open('inc-sr-varying-eps-cpu.json', 'w') as ofs:
            ofs.write(json.dumps({
                inc_sr_tag: {
                    '0.6':
                        inc_sr_varying_eps_cpu

                }
            }, indent=4))
        with open('inc-sr-varying-eps-mem.json', 'w') as ofs:
            ofs.write(json.dumps({
                inc_sr_tag: {
                    '0.6':
                        inc_sr_varying_eps_mem

                }
            }, indent=4))
        with open('inc-sr-varying-eps-cpu.json', 'w') as ofs:
            ofs.write(json.dumps({
                inc_sr_tag: {
                    '0.6':
                        inc_sr_varying_eps_cpu

                }
            }, indent=4))
        with open('inc-sr-varying-c-cpu.json', 'w') as ofs:
            ofs.write(json.dumps({
                inc_sr_tag: {
                    '0.01':
                        inc_sr_varying_c_cpu

                }
            }, indent=4))
        with open('inc-sr-varying-c-mem.json', 'w') as ofs:
            ofs.write(json.dumps({
                inc_sr_tag: {
                    '0.01':
                        inc_sr_varying_c_mem
                  
                }
            }, indent=4))
