#ifndef CBTREE_ADAPTER_H
#define CBTREE_ADAPTER_H

#include <iostream>
#include <sstream>
#include "cbtree_impl.h"
#include "common/errors.h"
#include "common/random_fnv1a.h"

#define DATA_STRUCTURE_T CBTree< K, V >

template <typename K, typename V, class Reclaim = K, class Alloc = K, class Pool = K>
class ds_adapter {
private:
    const V NO_VALUE;
    DATA_STRUCTURE_T * const tree;
public:
    ds_adapter(const int NUM_THREADS,
               const K& unused1,
               const K& KEY_POS_INFTY,
               const V& VALUE_RESERVED,
               RandomFNV1A * const unused2)
    : NO_VALUE(VALUE_RESERVED)
    , tree(new DATA_STRUCTURE_T(KEY_POS_INFTY, NO_VALUE))
    {}
    ~ds_adapter() {
        delete tree;
    }

    V getNoValue() {
        return NO_VALUE;
    }

    void initThread(const int tid) {
        assert(tid == 0);
    }
    void deinitThread(const int tid) {
        assert(tid == 0);
    }

    bool contains(const int tid, const K& key) {
        return tree->find(tid, key) != getNoValue();
    }
    V insert(const int tid, const K& key, const V& val) {
        setbench_error("insert-replace not implemented for this data structure");
    }
    V insertIfAbsent(const int tid, const K& key, const V& val) {
        return tree->insertIfAbsent(key, val);
    }
    V erase(const int tid, const K& key) {
        setbench_error("erase not implemented for this data structure");
    }
    V find(const int tid, const K& key) {
        return tree->find(key);
    }
    int rangeQuery(const int tid, const K& lo, const K& hi, K * const resultKeys, V * const resultValues) {
        setbench_error("rangeQuery not implemented for this data structure");
    }
    void printSummary() {
        //tree->stats();
    }
    bool validateStructure() {
        return true;
    }
    void printObjectSizes() {
        std::cout<<"sizes: node="
                 <<(sizeof(CBTreeNode<K, V>))
                 <<std::endl;
    }

};

#undef DATA_STRUCTURE_T

#endif