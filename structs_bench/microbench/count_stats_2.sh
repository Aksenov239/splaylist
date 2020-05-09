#rm -r results
mkdir results
make all
# echo "counting block for 100 100"
# ./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/flex100_100
# ./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results2/skip100_100
# ./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 ideal ideal > results2/ideal100_100

#seconds=(10)
#prefill=100000
#yx=(1 5 10)
#thr=(1 2 4 8 10 20 30 40 50 60 70)
#for((cnt=0;cnt<10;cnt++)) 
#do
#    for y in "${yx[@]}" 
#    do
#        for threads in "${thr[@]}" 
#        do
#            for s in "${seconds[@]}"
#            do
#                x=$((100-y))
#                echo "starting to count skiplist x=$x, y=$y, threads=$threads, secs=$s"
#                cops=$((100*threads))
#                # no adjustment
#                numactl --interleave=all ./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/skiplist
#            done        
#        done
#    done
#done

seconds=(10)
prefill=100000
yx=(1 5 10 100)
thr=(1 2 4 8 10 20 30 40 50 60 70)
#thr=(1)
#thr=(1 2)
for((cnt=0;cnt<10;cnt++)) 
do
    for y in "${yx[@]}" 
    do
        for threads in "${thr[@]}" 
        do
            for s in "${seconds[@]}"
            do
                x=$((100-y))
                if [ $y -eq 100 ]
                then
                    x=$((100))
                fi
                echo "starting to count x=$x, y=$y, threads=$threads, secs=$s"
                cops=$((100*threads))
                # no adjustment
                LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/skiplist_locks
                LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_skiplist_lazy.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/skiplist_lazy
#                numactl --interleave=all ./bin/bench_cbtree-ideal.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/tree

                # warmup + contains
#                numactl --interleave=all ./bin/bench_cbtree-ideal.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs $s x $x y $y cops $cops threads $threads >> results/cbtree_ideal
#                numactl --interleave=all ./bin/bench_flexlist-ideal.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs $s x $x y $y cops $cops threads $threads >> results/flexlist_ideal

                cops=$((1000*threads))
                LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/flexlist_1000t
                LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_cbtree-concurrent-fair.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/cbtree_fair_1000t

                cops=$((100*threads))
                #./bin/bench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs $s x $x y $y cops $cops threads $threads ideal ideal >> results/ideal_x_${x}_y_${y}_threads_${threads}_cops_${cops}_secs_${s}                cops=$((100*threads))
                LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/flexlist_100t
                LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_cbtree-concurrent-fair.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/cbtree_fair_100t

                cops=$((10*threads))
                LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/flexlist_10t
                LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_cbtree-concurrent-fair.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/cbtree_fair_10t


                cops=1000
                LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_cbtree-concurrent.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/cbtree_1000
#                numactl --interleave=all ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/flexlist_1000

                cops=100
                LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_cbtree-concurrent.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/cbtree_100

                cops=10
                LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/bench_cbtree-concurrent.alloc_new.reclaim_none.pool_none.out prefill $prefill secs $s presecs 0 x $x y $y cops $cops threads $threads >> results/cbtree_10
            done        
        done
    done
done    