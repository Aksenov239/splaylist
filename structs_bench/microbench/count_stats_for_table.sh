rm -r results_for_table
mkdir results_for_table
make all
# echo "counting block for 100 100"
# ./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/flex100_100
# ./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/skip100_100
# ./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 ideal ideal > results2/ideal100_100

seconds=(10) 
cops=(1 2 5 10 30 100)
prefill=100000
yx=(1 5 10)
thr=(1)
for((cnt=0;cnt<5;cnt++)) 
do
    for y in "${yx[@]}" 
    do
        for threads in "${thr[@]}" 
        do
            for s in "${seconds[@]}"
            do
                x=$((100-y))
                echo "starting to count x=$x, y=$y, threads=$threads, secs=$s"
                ./bin/ubench_cbtree.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs $s secs $s x $x y $y threads $threads >> results_for_table/cbtree
                ./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs $s secs $s x $x y $y threads $threads >> results_for_table/skip
#                ./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs $s secs $s x $x y $y threads $threads ideal ideal >> results_for_table/ideal
                for c in "${cops[@]}" 
                do
                    
                    echo "starting to count x=$x, y=$y, threads=$threads, secs=$s, cops=$c"
                    ./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs $s secs $s x $x y $y cops $c threads $threads >> results_for_table/flex
                done
            done        
        done
    done
done 