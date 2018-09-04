## Overview

This is the source code of the VLDB18 paper "ProbeSim: Scalable Single Source and Top-k SimRank Computations on Dynamic Graphs".
For any question of the code, please feel free to contact me. (Yu Liu, foreverhellas@163.com)

## File Organization

files | info
--- | ---
[pooling.h](pooling.h), [evaluate.h](evaluate.h) | four metrics in evaluation, and pooling eval-method borrowed from IR
[util.h](util.h), [Graph.h](Graph.h) | file utilities and graph loader
[Random.h](Random.h) | wrapper of SFMT (rand-gen with vectorization)
[SimStruct.h](SimStruct.h) | ProbeSim algorithms, deterministic, batched, randomized, pruning

## Requirement

architecture (at least):  `-march=core2` for the usage of SFMT (random number generator with vectorization)

## With Cmake Build & Run

```zsh
mkdir -p build && cd build
cmake .. && make -j
```

## Backup Without Cmake

### How to compile:

```zsh
sfmt_dir=directory of SFMT
g++ -march=core2 -pthread -std=c++11 -DSFMT_MEXP=607 -I $sfmt_dir -O3 -o ProbeSim sfmt_dir/SFMT.c main.cpp
```

#### How to run:

```zsh
./ProbeSim [-g <graph name>] [-e <eps, default 0.1>] [-q <query set>] [-d <output dir>]
```