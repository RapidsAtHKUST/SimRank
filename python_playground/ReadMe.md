## Shell To Convert File

### Steps

* sort 

```zsh
tail --lines=+2 out.digg-friends | sort -n -k4  > time_series_sorted_digg.txt
```

* awk (select `$1`, `$2`)

```zsh
cat time_series_sorted_digg.txt | awk '{print $1, $2}' > edge_list_digg.txt
```

in total with one instruction:

```zsh
tail --lines=+2 out.digg-friends | sort -n -k4 | awk '{print $1, $2}' > digg-friends.txt
tail --lines=+2 out.flickr-growth | sort -n -k4 | awk '{print $1, $2}' > flickr-growth.txt
```

## Reordering

reordering (`0` to `(n-1)`): [data.py](data/data.py)

make sure to put files in the folder './data/datasets/'

```zsh
cd data 
python data.py
```