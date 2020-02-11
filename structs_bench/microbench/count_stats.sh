mkdir results
make all
echo "counting block for 100 100"
./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results/out_flex
./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 > results/out_skip
./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 100 y 100 ideal ideal > results/out_ideal

echo "counting block for 90 10"
./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 1 x 90 y 10 > results/out_flex_90_10
./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 1 x 90 y 10 > results/out_skip_90_10
./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 1 x 90 y 10 ideal ideal > results/out_ideal_90_10
echo "start 10 seconds"
./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 90 y 10 >> results/out_flex_90_10
./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 90 y 10 >> results/out_skip_90_10
./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 90 y 10 ideal ideal >> results/out_ideal_90_10
echo "start 100 seconds"
./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 100 x 90 y 10 >> results/out_flex_90_10
./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 100 x 90 y 10 >> results/out_skip_90_10
./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 100 x 90 y 10 ideal ideal >> results/out_ideal_90_10

echo "counting block for 99 1"
./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 1 x 99 y 1 > results/out_flex_99_1
./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 1 x 99 y 1 > results/out_skip_99_1
./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 1 x 99 y 1 ideal ideal > results/out_ideal_99_1
echo "start 10 seconds"
./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 99 y 1 >> results/out_flex_99_1
./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 99 y 1 >> results/out_skip_99_1
./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 x 99 y 1 ideal ideal >> results/out_ideal_99_1
echo "start 100 seconds"
./bin/ubench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 100 x 99 y 1 >> results/out_flex_99_1
./bin/ubench_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 100 x 99 y 1 >> results/out_skip_99_1
./bin/ubench_ideal_skiplist_locks.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 100 x 99 y 1 ideal ideal >> results/out_ideal_99_1