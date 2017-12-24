## Dir Organization

util | files
--- | ---
graph(bidirectional csr) |
sfmt(simd fast rand gen) | 
file serialization | 
profiling | 

single-pair algorithm | files
--- | ---
bprw | 
flpmc |
bflpmc | 

all pair algorithms | files
--- | ---
local push | 
iterative(ground truth) | 

## Build

```zsh
mkdir build
cd build
cmake ..
make -j
```
