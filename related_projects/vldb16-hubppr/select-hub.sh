mkdir $2/global_pr/
./hubppr select-forward-hub --algo hubppr --prefix $2 --dataset $1 --epsilon 0.5 
./hubppr select-backward-hub --algo hubppr --prefix $2 --dataset $1 --epsilon 0.5 
