rm -r data_10seconds_vs_10minutes
mkdir data_10seconds_vs_10minutes
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
for((cnt=0;cnt<10;cnt++)) 
do
    echo "starting to count zipf, threads=1, secs=600, cops=100"
    if [ $cnt -lt 1 ]
    then
      ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs 600 cops 100 threads 1 zipf 1 >> data_10seconds_vs_10minutes/flexlist_10m_zipf
    fi
    echo "end of counting zipf, threads=1, secs=600, cops=100"
    ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs 10 cops 100 threads 1 zipf 1 >> data_10seconds_vs_10minutes/flexlist_10s_zipf
    # for y in "${yx[@]}" 
    # do
    #     for threads in "${thr[@]}" 
    #     do
    #         for s in "${seconds[@]}"
    #         do
    #             x=$((100-y))
    #             for c in "${cops[@]}" 
    #             do
    #                 echo "starting to count x=$x, y=$y, threads=$threads, secs=600, cops=$c"
    #                 if [ $cnt -lt 1 ]
    #                 then
    #                   ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs 600 x $x y $y cops $c threads $threads >> data_10seconds_vs_10minutes/flexlist_10m
    #                 fi
    #                 echo "end of counting x=$x, y=$y, threads=$threads, secs=600, cops=$c"
    #                 echo "starting to count x=$x, y=$y, threads=$threads, secs=$s, cops=$c"
    #                 ./bin/bench_flexlist_locks.alloc_new.reclaim_none.pool_none.out prefill $prefill presecs 0 secs $s x $x y $y cops $c threads $threads >> data_10seconds_vs_10minutes/flexlist_10s
    #             done
    #         done        
    #     done
    # done
done 