#! /bin/bash

LEN=(128 512 2048)
N_THR=(2 4 8 16 32 64 128)
N_ITR=(5)

echo "Board Width, Board Height, N. Threads, N. Iters, Seq Time, Par Time, FF Time, Par Speed Up, FF Speed Up, Par Scalability, FF Scalability, Par Efficiency, FF Efficiency" > $1

for l in ${LEN[@]}; do
    for nw in ${N_THR[@]}; do
        for itr in ${N_ITR[@]}; do
            ./ca -w $l -h 200 -i $itr -t $nw -I 5 --rand -s 2 >> $1
        done
    done
done
