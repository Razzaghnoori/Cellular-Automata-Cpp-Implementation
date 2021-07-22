#! /bin/bash

LEN=(30 100 300 1000)
N_THR=(2 4 8)
N_ITR=(2 8 32)

echo "Board Width, Board Height, N. Row Threads, N. Col Threads, N. Iters, Seq Time, Par Time, FF Time, Par Speed Up, FF Speed Up, Par Scalability, FF Scalability, Par Efficiency, FF Efficiency" > $1

for l in ${LEN[@]}; do
    for nw in ${N_THR[@]}; do
        for itr in ${N_ITR[@]}; do
            ./ca -w $l -h $l -i $itr -r $nw -c $nw -I 5 --rand >> $1
        done
    done
done