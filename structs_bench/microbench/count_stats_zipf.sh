rm -r results_zipf
mkdir results_zipf
make all
# echo "counting block for 100 100"
# ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/flex100_100
# ./bin/bench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/skip100_100
# ./bin/bench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 ideal ideal > results2/ideal100_100

seconds=(10)
prefill=10000000
thr=(1 2 4 8 10 20 30 40 50 60 70)
#thr=(70)
for((cnt=0;cnt<10;cnt++)) 
do
    for threads in "${thr[@]}" 
    do
        for s in "${seconds[@]}"
        do
            echo "starting to count threads=$threads, secs=$s"

            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 threads $threads >> results_zipf/skiplist_locks
#            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_skiplist_lazy.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/skiplist_lazy

            cops=$((1000*threads))
#            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/flexlist_1000t
            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_flexlist_locks_64_paddings.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/flexlist_64_1000t
            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_cbtree-concurrent-fair.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/cbtree_fair_1000t

            cops=$((100*threads))
#            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/flexlist_100t
            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_flexlist_locks_64_paddings.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/flexlist_64_100t
            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_cbtree-concurrent-fair.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/cbtree_fair_100t

            cops=$((10*threads))
#            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/flexlist_10t
            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_flexlist_locks_64_paddings.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/flexlist_64_10t
            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_cbtree-concurrent-fair.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/cbtree_fair_10t

            cops=1000
            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_cbtree-concurrent.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/cbtree_1000

            cops=100
            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_cbtree-concurrent.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/cbtree_100

            cops=10
            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_cbtree-concurrent.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 zipf 1 cops $cops threads $threads >> results_zipf/cbtree_10
        done        
    done
done    