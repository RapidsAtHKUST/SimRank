Code Contributors: Xiaodong He

If you have any questions, feel free to contact me. My emails are: 2011202413@ruc.edu.cn

Please cite our paper if you choose to use our code. 

```
@inproceedings{wei2018topppr,
  title        = {Topppr: top-k personalized pagerank queries with precision guarantees on large graphs},
  author       = {Wei, Zhewei and
                  He, Xiaodong and
                  Xiao, Xiaokui and
                  Wang, Sibo and
                  Shang, Shuo and
                  Wen, Ji-Rong},
  booktitle    = {Proceedings of the 2018 International Conference on Management of Data},
  pages        = {441--456},
  year         = {2018},
  organization = {ACM}
}
```

# Single Source Personalized PageRank

## Tested Environment
- Ubuntu
- C++ 11
- GCC 4.8

## Preparation
Place the prepared data [file].txt in the ./dataset folder and create the subfolder [file] with the same name in the ppr-answer folder. Note that the first row of data is the node size, and each row is the information of each edge [outNode] [inNode]. test.txt is example.

## Compile
```sh
sh compile.sh
```

## Parameters
```sh
./TopPPR --algo <algorithm> [options]
```
- algo:
    - GEN_QUERY: generate query node file
    - GEN_GROUND_TRUTH: generate query node groundtruth by multi-thread power method
    - TopPPR: TopPPR query
- options
    - -d \<dataset\>
    - -k \<top k\>
    - -n \<query size\>
    - -r \<precision threshold\>
    - -err \<max error (defalt 10^-10)\>
    - -a \<alpha (default 0.2)\>

## Generate queries
Generate query files for the graph data. 

- Example:

```sh
$ sh gen_query.sh
```

## Generate groundtruth of query nodes

- Example:

```sh
$ sh gen_groundtruth.sh
```

## Exact Query

- Example:

```sh
./TopPPR -d test -algo TopPPR -r 1 -n 50 -k 5
```

## Approximate Query

- Example:

```sh
./TopPPR -d test -algo TopPPR -r 0.4 -n 50 -k 5
```
