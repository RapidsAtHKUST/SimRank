## Yche Experiments

```zsh
./simpush -f /mnt/storage-praha16/dataset/lj.e.bin -qn 5

```

## Legacy

* Run:

```
make
./simpush -f dblp -qn 50
```

* Evaluation:

```
g++ -std=c++11 cal_evalaute.cpp -o eval
bash run_dblp_eval.sh
```
