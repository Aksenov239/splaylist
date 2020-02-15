rm -r results
mkdir results
make all
# echo "counting block for 100 100"
# ./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/flex100_100
# ./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/skip100_100
# ./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 ideal ideal > results2/ideal100_100

seconds=(10)
prefill=100000
yx=(1 5 10)
thr=(1 2 4 8 10 20 30 40 50 60 70)
#thr=(1 2)
for((cnt=0;cnt<5;cnt++)) 
do
    for y in "${yx[@]}" 
    do
        for threads in "${thr[@]}" 
        do
            for s in "${seconds[@]}"
            do
                x=$((100-y))
                cops=$((100*threads))
                echo "starting to count x=$x, y=$y, threads=$threads, secs=$s, cops=$cops"
                numactl --interleave=all ./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/skip_x_${x}_y_${y}_threads_${threads}_cops_${cops}_secs_${s}
                #./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs $s x $x y $y cops $cops threads $threads ideal ideal >> results/ideal_x_${x}_y_${y}_threads_${threads}_cops_${cops}_secs_${s}
                numactl --interleave=all ./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/flex_x_${x}_y_${y}_threads_${threads}_cops_${cops}_secs_${s}
            done        
        done
    done
done    