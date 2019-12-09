for ((hub=0;hub<=10000000;hub+=2000000)); do
    for ((i=0;i<3;i+=1)); do
        echo "hub ${hub}, trial ${i}"
        /homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-fgi/build/carmo-h ca-GrQc 100000 0 500 0.01 ${hub}
        echo ""
    done
done
