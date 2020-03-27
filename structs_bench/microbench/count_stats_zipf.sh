#rm -r results
mkdir results_zipf
make all
# echo "counting block for 100 100"
# ./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/flex100_100
# ./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/skip100_100
# ./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 ideal ideal > results2/ideal100_100

seconds=(10)
prefill=100000
thr=(1 2 4 8 10 20 30 40 50 60 70)
for((cnt=0;cnt<5;cnt++)) 
do
    for threads in "${thr[@]}" 
    do
        for s in "${seconds[@]}"
        do
            echo "starting to count threads=$threads, secs=$s"

            cops=$((1000*threads))
            numactl --interleave=all ./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/flexlist_1000t
            numactl --interleave=all ./bin/ubench_cbtree-concurrent-fair.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/cbtree_fair_1000t

            cops=$((100*threads))
            numactl --interleave=all ./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/flexlist_100t
            numactl --interleave=all ./bin/ubench_cbtree-concurrent-fair.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/cbtree_fair_100t

            cops=$((10*threads))
            numactl --interleave=all ./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/flexlist_10t
            numactl --interleave=all ./bin/ubench_cbtree-concurrent-fair.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/cbtree_fair_10t

            cops=1000
            numactl --interleave=all ./bin/ubench_cbtree-concurrent.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/cbtree_1000

            cops=100
            numactl --interleave=all ./bin/ubench_cbtree-concurrent.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/cbtree_100

            cops=10
            numactl --interleave=all ./bin/ubench_cbtree-concurrent.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/cbtree_10
        done        
    done
done    