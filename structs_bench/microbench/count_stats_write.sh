#rm -r results
mkdir results
make all

seconds=(10)
prefill=100000
yx=(1 5 10 100)
pw=(0 0.02 0.1 0.2)
for pwrite in "${pw[@]}" 
do
    for y in "${yx[@]}" 
    do
        x=$((100-y))
        if [ $y -eq 100 ]
        then
            x=$((100))
        fi
        for s in "${seconds[@]}"
        do
            for((cnt=0;cnt<5;cnt++)) 
            do
                echo "starting to count pwrite=$pwrite, x=$x, y=$y, secs=$s"
                cops=7000
#                LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/write_flexlist_locks_64_paddings.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 presecs 0 x $x y $y cops 7000 write $pwrite writeset 0.25 threads 70 >> results/$pwrite
            done        
        done
    done
done    

for pwrite in "${pw[@]}" 
do
    for s in "${seconds[@]}"
    do
        for((cnt=0;cnt<5;cnt++)) 
        do
            echo "starting to count pwrite=$pwrite, Zipf 1, secs=$s"
            cops=7000
            LD_PRELOAD=../lib/jemalloc/lib/libjemalloc.so numactl --interleave=all ./bin/write_flexlist_locks_64_paddings.alloc_new.reclaim_none.pool_none.out prefill 100000 secs 10 presecs 0 zipf 1 cops 7000 write $pwrite writeset 0.25 threads 70 >> results/$pwrite
        done        
    done
done    