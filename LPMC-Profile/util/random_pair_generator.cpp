//
// Created by yche on 1/17/18.
//
#include <iostream>

#include "random_pair_generator.h"

#include "../playground/pretty_print.h"

void generate_sample_pairs(string graph_name, int pair_num, int round_num) {
    string path = get_edge_list_path(graph_name);
    GraphYche g(path);

    // generate pairs
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> distribution(0, g.n - 1);

    for (auto round_i = 0; round_i < round_num; round_i++) {
        vector<pair<int, int>> sample_pairs;
sample_pairs.emplace_back(4847567, 4847566);
sample_pairs.emplace_back(4847566, 4847565);
sample_pairs.emplace_back(4847554, 4847553);
sample_pairs.emplace_back(4847549, 4847548);
sample_pairs.emplace_back(4847548, 4847547);
sample_pairs.emplace_back(4847547, 4847546);
sample_pairs.emplace_back(4847538, 4847537);
sample_pairs.emplace_back(4847537, 4847536);
sample_pairs.emplace_back(4847536, 4847534);
sample_pairs.emplace_back(4847535, 4847532);
sample_pairs.emplace_back(4847532, 4847531);
sample_pairs.emplace_back(4847529, 4847528);
sample_pairs.emplace_back(4847515, 4847514);
sample_pairs.emplace_back(4847509, 4847508);
sample_pairs.emplace_back(4847502, 4847487);
sample_pairs.emplace_back(4847497, 4847207);
sample_pairs.emplace_back(4847492, 4847491);
sample_pairs.emplace_back(4847487, 4847486);
sample_pairs.emplace_back(4847478, 4847477);
sample_pairs.emplace_back(4847473, 4847460);
sample_pairs.emplace_back(4847459, 4847418);
sample_pairs.emplace_back(4847454, 4847451);
sample_pairs.emplace_back(4847451, 4847413);
sample_pairs.emplace_back(4847446, 4847380);
sample_pairs.emplace_back(4847443, 4847373);
sample_pairs.emplace_back(4847441, 4847373);
sample_pairs.emplace_back(4847433, 4847358);
sample_pairs.emplace_back(4847428, 4847426);
sample_pairs.emplace_back(4847425, 4847409);
sample_pairs.emplace_back(4847424, 4847423);
sample_pairs.emplace_back(4847420, 4847419);
sample_pairs.emplace_back(4847418, 4847408);
sample_pairs.emplace_back(4847413, 4847412);
sample_pairs.emplace_back(4847411, 4847410);
sample_pairs.emplace_back(4847409, 4846836);
sample_pairs.emplace_back(4847408, 4847407);
sample_pairs.emplace_back(4847407, 4847406);
sample_pairs.emplace_back(4847406, 4847405);
sample_pairs.emplace_back(4847398, 4847395);
sample_pairs.emplace_back(4847383, 4847382);
sample_pairs.emplace_back(4847381, 4847379);
sample_pairs.emplace_back(4847374, 4847372);
sample_pairs.emplace_back(4847373, 4847369);
sample_pairs.emplace_back(4847372, 4847371);
sample_pairs.emplace_back(4847371, 4847370);
sample_pairs.emplace_back(4847369, 4847367);
sample_pairs.emplace_back(4847367, 4847366);
sample_pairs.emplace_back(4847366, 4847361);
sample_pairs.emplace_back(4847365, 4847358);
sample_pairs.emplace_back(4847360, 4847359);
sample_pairs.emplace_back(4847358, 4847350);
sample_pairs.emplace_back(4847356, 4847355);
sample_pairs.emplace_back(4847347, 4847346);
sample_pairs.emplace_back(4847346, 4847306);
sample_pairs.emplace_back(4847344, 4847343);
sample_pairs.emplace_back(4847343, 4847342);
sample_pairs.emplace_back(4847328, 4847327);
sample_pairs.emplace_back(4847326, 4847325);
sample_pairs.emplace_back(4847325, 4847321);
sample_pairs.emplace_back(4847321, 4847263);
sample_pairs.emplace_back(4847319, 4847316);
sample_pairs.emplace_back(4847316, 4847313);
sample_pairs.emplace_back(4847302, 4847301);
sample_pairs.emplace_back(4847297, 4847296);
sample_pairs.emplace_back(4847290, 4847289);
sample_pairs.emplace_back(4847289, 4847288);
sample_pairs.emplace_back(4847287, 4847249);
sample_pairs.emplace_back(4847286, 4847279);
sample_pairs.emplace_back(4847283, 4847282);
sample_pairs.emplace_back(4847279, 4847277);
sample_pairs.emplace_back(4847277, 4847271);
sample_pairs.emplace_back(4847272, 4847270);
sample_pairs.emplace_back(4847271, 4847269);
sample_pairs.emplace_back(4847269, 4847268);
sample_pairs.emplace_back(4847268, 4847267);
sample_pairs.emplace_back(4847267, 4847265);
sample_pairs.emplace_back(4847265, 4847255);
sample_pairs.emplace_back(4847255, 4847250);
sample_pairs.emplace_back(4847251, 4847236);
sample_pairs.emplace_back(4847250, 4847243);
sample_pairs.emplace_back(4847243, 4847241);
sample_pairs.emplace_back(4847241, 4847239);
sample_pairs.emplace_back(4847239, 4847238);
sample_pairs.emplace_back(4847238, 4847237);
sample_pairs.emplace_back(4847237, 4846662);
sample_pairs.emplace_back(4847235, 4847234);
sample_pairs.emplace_back(4847234, 4847233);
sample_pairs.emplace_back(4847233, 4847230);
sample_pairs.emplace_back(4847231, 4847230);
sample_pairs.emplace_back(4847230, 4847227);
sample_pairs.emplace_back(4847227, 4847183);
sample_pairs.emplace_back(4847224, 4847223);
sample_pairs.emplace_back(4847198, 4847197);
sample_pairs.emplace_back(4847197, 4847196);
sample_pairs.emplace_back(4847188, 4847186);
sample_pairs.emplace_back(4847186, 4847185);
sample_pairs.emplace_back(4847183, 4847182);
sample_pairs.emplace_back(4847177, 4847175);
sample_pairs.emplace_back(4847165, 4847155);
sample_pairs.emplace_back(4847161, 4847160);

        for (auto i = 0; i < pair_num - 100; i++) {
            auto u = distribution(gen);
            auto v = distribution(gen);
            while (u == v) {
                u = distribution(gen);
                v = distribution(gen);
            }
            sample_pairs.emplace_back(u, v);
        }
#ifdef DEBUG
        cout << sample_pairs << endl;
        cout << get_file_path(graph_name, round_i, pair_num) << endl;
#endif

        // string ofs_file_path = get_file_path(graph_name, round_i, pair_num);
        string ofs_file_path = get_file_path(graph_name, 42, pair_num);
        FILE *ofs = fopen(ofs_file_path.c_str(), "wb");
        YcheSerializer serializer;
        serializer.write_vec(ofs, sample_pairs);
        fclose(ofs);
    }
}

vector<pair<unsigned int, unsigned int>> read_sample_pairs(string graph_name, int pair_num, int round) {
    vector<pair<unsigned int, unsigned int>> sample_pairs;
    string ifs_file_path = get_file_path(graph_name, round, pair_num);

    FILE *ifs = fopen(ifs_file_path.c_str(), "r");
    YcheSerializer serializer;
    serializer.read_vec(ifs, sample_pairs);
    fclose(ifs);
    return sample_pairs;
};
