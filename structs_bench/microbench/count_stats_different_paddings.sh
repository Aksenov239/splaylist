# rm -r data_different_paddings
# mkdir data_different_paddings
make -j4 all
# echo "counting block for 100 100"
# ./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/flex100_100
# ./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/skip100_100
# ./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 ideal ideal > results2/ideal100_100

seconds=(10) 
cops=(7000)
prefill=100000
yx=(1 5 10)
thr=(70)
for((cnt=0;cnt<10;cnt++)) 
do
    for y in "${yx[@]}" 
    do
        for threads in "${thr[@]}" 
        do
            for s in "${seconds[@]}"
            do
                x=$((100-y))
                for c in "${cops[@]}" 
                do
                    echo "starting to count x=$x, y=$y, threads=$threads, secs=$s, cops=$c"
                    if [ $y -lt 3 ]
                    then
                        ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs $s cops $c threads $threads zipf 1 >> data_different_paddings/flexlist_10s_70_128_zipf
                        ./bin/bench_flexlist_locks_64_paddings.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs $s cops $c threads $threads zipf 1 >> data_different_paddings/flexlist_10s_70_64_zipf
                        ./bin/bench_flexlist_locks_nopaddings.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs $s cops $c threads $threads zipf 1 >> data_different_paddings/flexlist_10s_70_0_zipf
                    fi
                    ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs $s x $x y $y cops $c threads $threads >> "data_different_paddings/flexlist_10s_70_128_x_$x"
                    ./bin/bench_flexlist_locks_64_paddings.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs $s x $x y $y cops $c threads $threads >> "data_different_paddings/flexlist_10s_70_64_x_$x"
                    ./bin/bench_flexlist_locks_nopaddings.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs $s x $x y $y cops $c threads $threads >> "data_different_paddings/flexlist_10s_70_0_x_$x"
                done
            done        
        done
    done
done 