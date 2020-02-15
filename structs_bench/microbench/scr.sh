for ((threads=1;threads<=7;threads++)) 
do 
	./bin/ubench_cbtree-concurrent.alloc_new.reclaim_none.pool_none.out threads $threads presecs 0 secs 10 x 99 y 1  prefill 100000 >>cbtree_dif
done;