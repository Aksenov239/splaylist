/**
 * @TODO: ADD COMMENT HEADER
 */

#ifndef BST_ADAPTER_H
#define BST_ADAPTER_H

#include <iostream>
#include <csignal>
#include "errors.h"
#include "random_fnv1a.h"
#include "record_manager.h"
#ifdef USE_TREE_STATS
#   include "tree_stats.h"
#endif
#include "flexlist.h"
//TODO: understand about get into receive manager

#define RECORD_MANAGER_T record_manager<Reclaim, Alloc, Pool, Node<K, V>>
#define DATA_STRUCTURE_T FlexList<K, V, RECORD_MANAGER_T>


template <typename K, typename V, class Reclaim = reclaimer_debra<K>, class Alloc = allocator_new<K>, class Pool = pool_none<K>>
class ds_adapter {
private:
    const V NO_VALUE;
    DATA_STRUCTURE_T * const ds;

public:
    ds_adapter(const int NUM_THREADS,
               const K& KEY_MIN,
               const K& KEY_MAX,
               const V& VALUE_RESERVED,
               RandomFNV1A * const unused2)
            : NO_VALUE(VALUE_RESERVED)
            , ds(new DATA_STRUCTURE_T(NUM_THREADS, VALUE_RESERVED, KEY_MIN, KEY_MAX))
    { }

    ~ds_adapter() {
        delete ds;
    }

    V getNoValue() {
        return 0;
    }

    void initThread(const int tid) {
        ds->initThread(tid);
    }
    void deinitThread(const int tid) {
        ds->deinitThread(tid);
    }
    void setCops(const int tid, int cops) {
        ds->setCops(cops);
    }

    V insert(const int tid, const K& key, const V& val) {
        setbench_error("insert-replace functionality not implemented for this data structure");
    }

    V insertIfAbsent(const int tid, const K& key, const V& val) {
        return ds->insertIfAbsent(tid, key, val);
    }

    V erase(const int tid, const K& key) {
        setbench_error("erase functionality not implemented for this data structure");
    }

    V find(const int tid, const K& key) {
        return ds->qfind(tid, key);
    }

    long long getPathsLength(const int tid) {
        return ds->getPathsLength(tid);
    }

    bool contains(const int tid, const K& key) {
        return ds->contains(tid, key);
    }
    int rangeQuery(const int tid, const K& lo, const K& hi, K * const resultKeys, V * const resultValues) {
        setbench_error("not implemented");
    }
    void printSummary() {
        ds->printDebuggingDetails();
    }
    bool validateStructure() {
        return ds->validate();
    }
    int getHeight() {
        return ds->getHeight();
    }

    void printObjectSizes() {
        std::cout<<"sizes: node="
                 <<(sizeof(Node<K, V>))
                 <<std::endl;
    }

#ifdef USE_TREE_STATS
    class NodeHandler {
    public:
        typedef Node<K, V> * NodePtrType;
        K minKey;
        K maxKey;

        NodeHandler(const K& _minKey, const K& _maxKey) {
            minKey = _minKey;
            maxKey = _maxKey;
        }

        class ChildIterator {
        private:
            NodePtrType node; // node being iterated over
            bool done;
        public:
            ChildIterator(NodePtrType _node) {
                node = _node;
                done = false;
            }

            bool hasNext() {
                return node->next[0] != NULL && !done;
            }

            NodePtrType next() {
                done = true;
                return node->next[0];
            }
        };

        bool isLeaf(NodePtrType node) {
            return node->next[0] == NULL ? true : false;
        }
        size_t getNumChildren(NodePtrType node) {
            return node->next[0] == NULL ? 0 : 1;
        }
        size_t getNumKeys(NodePtrType node) {
            return node == NULL ? 0 : 1;
        }

        size_t getSumOfKeys(NodePtrType node) {
            return (size_t) node->key;
        }
        ChildIterator getChildIterator(NodePtrType node) {
            return ChildIterator(node);
        }
    };
    TreeStats<NodeHandler> * createTreeStats(const K& _minKey, const K& _maxKey) {
        return new TreeStats<NodeHandler>(new NodeHandler(_minKey, _maxKey), ds->getRoot(), false);
    }
#endif
};

#endif
