## Dir Organization

util | files
--- | ---
graph(bidirectional csr) | [graph_yche.h](util/graph_yche.h), [graph_yche.cpp](util/graph_yche.cpp)
spare matrix | [sparse_matrix_utils.h](util/sparse_matrix_utils.h)
sfmt(simd fast rand gen) | [sfmt_based_rand.h](util/sfmt_based_rand.h)
file serialization | [file_serialization.h](util/file_serialization.h)
profiling | [stat.h](util/stat.h), [stat.cpp](util/stat.cpp)

single-pair algorithm | files
--- | ---
bprw | [bprw_yche.h](yche_refactor/bprw_yche.h), [bprw_yche.cpp](yche_refactor/bprw_yche.cpp)
flpmc | [flpmc_yche.h](yche_refactor/flpmc_yche.h), [flpmc_yche.cpp](yche_refactor/flpmc_yche.cpp)
bflpmc | [bflpmc_yche.h](yche_refactor/bflpmc_yche.h), [bflpmc_yche.cpp](yche_refactor/bflpmc_yche.cpp)

all pair algorithms | files
--- | ---
local push(static) | [local_push_yche.h](yche_refactor/local_push_yche.h), [local_push_yche.cpp](yche_refactor/local_push_yche.cpp)
iterative(ground truth) | [simrank.h](yche_refactor/simrank.h), [simrank.cpp](yche_refactor/simrank.cpp)

## Build

```zsh
mkdir build
cd build
cmake ..
make -j
```
