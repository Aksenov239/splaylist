#!./bin/bash
g++ -std=c++11 -g ./gen.cpp -o ./gen
make all

for((i=0; i< 100000; i++)) do
    echo "$i"
    ./gen "$i" > input || break;
    for ((j=0; j < 20; j++)) do
        ./bin/flextest_flexlist_locks.alloc_new.reclaim_none.pool_none.out < input > out || break 2
        diff ./out ./out2 || break 2        
    done
done