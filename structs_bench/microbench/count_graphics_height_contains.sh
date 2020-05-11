rm -r data_graphics_height_contains
mkdir data_graphics_height_contains
make -j4 all
# echo "counting block for 100 100"
# ./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/flex100_100
# ./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/skip100_100
# ./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 ideal ideal > results2/ideal100_100

seconds=(10) 
cops=(100)
prefill=100000
yx=(1 5 10)
thr=(1)
for((cnt=0;cnt<1;cnt++)) 
do
    ./bin/stats_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs 10 cops 100 threads 1 zipf 1 >> data_graphics_height_contains/flexlist_contains_height_zipf
    for y in "${yx[@]}" 
    do
        for threads in "${thr[@]}" 
        do
            for s in "${seconds[@]}"
            do
                x=$((100-y))
                for c in "${cops[@]}" 
                do
                    ./bin/stats_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs $s x $x y $y cops $c threads $threads >> "data_graphics_height_contains/flexlist_contains_height_x_$x_y_$y"
                done
            done        
        done
    done
done 