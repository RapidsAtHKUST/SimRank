#!/bin/bash
make
./hubppr generate-p2p-query --algo hubppr --prefix $2 --dataset $1  --target_sample uniform
./hubppr generate-topk-query --algo hubppr --prefix $2 --dataset $1  --target_size 1600
