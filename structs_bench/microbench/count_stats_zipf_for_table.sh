rm -r results_zipf_for_table
mkdir results_zipf_for_table
make all

seconds=(10) 
cops=(1 2 5 10 100 1000)
prefill=100000
thr=(1)
for((cnt=0;cnt<10;cnt++)) 
do
    for threads in "${thr[@]}" 
    do
        for s in "${seconds[@]}"
        do
            echo "starting to count threads=$threads, secs=$s"
            ./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs $s zipf 1 threads $threads >> results_zipf_for_table/skiplist
            for c in "${cops[@]}" 
            do
                echo "starting to count threads=$threads, secs=$s, cops=$c"
                ./bin/ubench_cbtree-concurrent.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs $s zipf 1 cops $c threads $threads >> results_zipf_for_table/cbtree_$c
                ./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs $s zipf 1 cops $c threads $threads >> results_zipf_for_table/flexlist_$c
            done
        done        
    done
done 