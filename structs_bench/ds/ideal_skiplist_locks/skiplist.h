//
// Created by demon1999 on 15.11.19.
//

#include <bits/stdc++.h>
#include <unordered_set>
#include "locks_impl.h"
#ifndef LOCK_BASED_SKIP_LIST_SKIP_LIST_H
#define LOCK_BASED_SKIP_LIST_SKIP_LIST_H
const int MAX_LEVEL = 25;
const int MAX_THREADS = 1000;
const int PADDING_SIZE = 128;

class PaddedRandom {
    volatile char pad0[PADDING_SIZE - sizeof(unsigned int)];
    unsigned int seed;
    //std::mt19937 rnd;
public:
    void setSeed(unsigned int _seed) {
        seed = _seed;
        //rnd = std::mt19937(_seed);
    }
    unsigned int genRandom() {
        seed ^= (seed << 6);
        seed ^= (seed >> 21);
        seed ^= (seed << 13);
        //uint32_t sd = rnd();
        //std::cout << sd % 16 << "\n";
        for (unsigned int i = 0; i < MAX_LEVEL; i++) {
            if ((seed & (1 << i)) == 0) {
                return i;
            }
        }
        return 0;
    }
};

template<typename K, typename V>
struct Node {
    K key;
    V value;
    bool mark;
    bool fullyLinked;
    volatile int lock;
    int topLevel;
    Node<K, V>* next[MAX_LEVEL + 1];
};

struct MyLength {
    long long value;
    volatile char pad[PADDING_SIZE];
    MyLength() {
        value = 0;
    }
};


template<typename K, typename V, class RecordManager>
class SkipList {
private:
    volatile char pad_4[PADDING_SIZE];
    volatile int size = 0;
    volatile char pad_3[PADDING_SIZE];
    volatile int realMaxLvl = 1;
    volatile char pad_2[PADDING_SIZE];
    MyLength sumLengths[MAX_THREADS];
    volatile char pad_1[PADDING_SIZE];
    Node<K, V>* head;
    volatile char pad0[PADDING_SIZE];
    Node<K, V>* tail;
    volatile char pad1[PADDING_SIZE];
    RecordManager * const recordManager;
    volatile char pad2[PADDING_SIZE];
    PaddedRandom rnd[MAX_THREADS];
    volatile char pad3[PADDING_SIZE];
    V noValue;
    volatile char pad4[PADDING_SIZE];
    int init[MAX_THREADS] = {0,};
    volatile char pad5[PADDING_SIZE];
public:
    SkipList(const int numThreads, const V noValue, const K minKey, const K maxKey);

    ~SkipList();

    Node<K, V>* createNode(const int tid, int h, K key, V value);

    bool contains(const int tid, const K &key);

    V insertIfAbsent(const int tid, const K &key, const V &value);

    V erase(const int tid, const K &key);

    bool validate();

    void printDebuggingDetails();

    void initThread(const int tid);

    void deinitThread(const int tid);

    long long getPathsLength(const int tid);

    int getHeight();

    Node<K, V>* getRoot();

private:
    std::pair<int, V> find(const int tid, K key, Node<K, V> **preds, Node<K, V> **succs);
    std::pair<int, V> find(const int tid, K key, Node<K, V> **preds, Node<K, V> **succs, int maxLevel);
};

template <typename K, typename V, class RecordManager>
Node<K, V>* SkipList<K, V, RecordManager>::createNode(const int tid, int h, K key, V value) {
    Node<K, V>* node = recordManager->template allocate<Node<K,V>>(tid);//new Node<K, V>();//
    node->topLevel = h;
    node->key = key;
    node->value = value;
    node->fullyLinked = false;
    node->mark = false;
    node->lock = 0;
    return node;
}

template <typename K, typename V, class RecordManager>
SkipList<K, V, RecordManager>::SkipList(const int numThreads, const V noValue, const K minKey, const K maxKey) :
    recordManager(new RecordManager(numThreads)) {
    const int tid = 0;
    initThread(tid);
    recordManager->endOp(tid);
    head = createNode(tid, MAX_LEVEL, minKey - 1, 0);
    tail = createNode(tid, MAX_LEVEL, maxKey + 1, 0);
    this->noValue = noValue;

    for (unsigned int i = 0; i < numThreads; i++) {
        sumLengths[i] = MyLength();
    }

    for (int i = 0; i <= MAX_LEVEL; i++) {
        head->next[i] = tail;
        tail->next[i] = NULL;
    }
    for (unsigned int i = 0; i < numThreads; i++) {
        rnd[i].setSeed(i + 1);
    }
}

template <typename K, typename V, class RecordManager>
void SkipList<K, V, RecordManager>::printDebuggingDetails() {
    for (int level = 0; level <= realMaxLvl; level ++) {
        std::cout << level << ": ";
        Node<K, V>* h = head;
        while (h != tail) {
            std::cout << h->key << " ";
            h = h->next[level];
        }
        std::cout << "\n";
    }
}

template <typename K, typename V, class RecordManager>
bool SkipList<K, V, RecordManager>::contains(const int tid, const K &key) {
    int flevel = -1;
    V val = NULL;
    Node<K, V>* pred = head;
    for (int level = realMaxLvl; level >= 0; level--) {
        Node<K, V>* curr = pred;
        while (key > curr->key) {
            sumLengths[tid].value++;
            pred = curr;
            curr = pred->next[level];
        }
        if (key == curr->key) {
            return (curr->fullyLinked) && (!curr->mark);
        }
    }
    assert(0);
    return false;
}

template <typename K, typename V, class RecordManager>
V SkipList<K, V, RecordManager>::insertIfAbsent(const int tid, const K &key, const V &value) {
    //TODO: retire for preds ans succs
    //std::cout << key << " " << topLevel << "\n";
    int c = __sync_add_and_fetch(&size, 1);
    int topLevel = 0;
    while ((c & 1) == 0) {
        topLevel++;
        c >>= 1;
    }
    while (true) {
        int k = realMaxLvl;
        if (topLevel <= k)
            break;
        if (__sync_bool_compare_and_swap(&realMaxLvl, k, topLevel)) {
            break;
        }
    }
    Node<K, V>* preds[MAX_LEVEL + 1];
    Node<K, V>* succs[MAX_LEVEL + 1];
    while (true) {
        auto pr = find(tid, key, preds, succs);
        if (pr.first != -1) {
            Node<K, V>* found = succs[pr.first];
            if (!(found->mark)) {
                while (!(found->fullyLinked)) {}
                return pr.second;
            }
            continue;
        }
        int lockLevel = -1;
        bool valid = true;
        for (int level = 0; level <= topLevel; level++) {
            Node<K, V>* pred = preds[level];
            Node<K, V>* succ = succs[level];
            if (level == 0 || pred != preds[level - 1]) {
                acquireLock(&(pred->lock));
            }
            lockLevel = level;
            valid = !pred->mark && !succ->mark && (pred->next[level] == succ);
            if (!valid)
                break;
        }
        if (!valid) {
            for (int level = 0; level <= lockLevel; level++) {
                if (level == 0 || preds[level] != preds[level - 1])
                    releaseLock(&(preds[level]->lock));
            }
            continue;
        }

        Node<K, V>* newNode = createNode(tid, topLevel, key, value);
        for (int level = 0; level <= topLevel; level++) {
            newNode->next[level] = succs[level];
        }
        for (int level = 0; level <= topLevel; level++) {
            preds[level]->next[level] = newNode;
        }
        newNode->fullyLinked = true;
        for (int level = 0; level <= lockLevel; level++) {
            if (level == 0 || preds[level] != preds[level - 1])
                releaseLock(&(preds[level]->lock));
        }
        return NULL;
    }
}

template <typename K, typename V, class RecordManager>
V SkipList<K, V, RecordManager>::erase(const int tid, const K &key) {
    //TODO: think about retire for node, where to put it and so on
    //TODO: retire for preds ans succs
    Node<K, V>* preds[MAX_LEVEL + 1];
    Node<K, V>* succs[MAX_LEVEL + 1];
    bool isMarked = false;
    Node<K, V>* victim = NULL;
    int topLevel = -1;
    while (true) {
        auto pr = find(tid, key, preds, succs);
        if (pr.first != -1) {
            victim = succs[pr.first];
        }
        if (isMarked || (pr.first != -1 && pr.first == victim->topLevel && !victim->mark)) {
            if (!isMarked) {
                topLevel = victim->topLevel;
                acquireLock(&(victim->lock));
                if (victim->mark) {
                    releaseLock(&(victim->lock));
                    return noValue;
                }
                isMarked = true;
                victim->mark = true;
            }
            int lockLevel = -1;
            bool valid = true;
            for (int level = 0; level <= topLevel; level++) {
                Node<K, V>* pred = preds[level];
                if (level == 0 || pred != preds[level - 1])
                    acquireLock(&(pred->lock));
                lockLevel = level;
                valid = !pred->mark && ((pred->next[level]) == victim);
                if (!valid)
                    break;
            }
            if (!valid) {
                for (int level = 0; level <= lockLevel; level++) {
                    if (level == 0 || preds[level] != preds[level - 1])
                        releaseLock(&(preds[level]->lock));
                }
                continue;
            }
            for (int level = topLevel; level >= 0; level--) {
                preds[level]->next[level] = victim->next[level];
            }
            for (int level = 0; level <= lockLevel; level++) {
                if (level == 0 || preds[level] != preds[level - 1])
                    releaseLock(&(preds[level]->lock));
            }
            releaseLock(&(victim->lock));
            V value = (victim->value);
            recordManager->retire(tid, victim);
            return value;
        } else {
            return noValue;
        }
    }
}

template <typename K, typename V, class RecordManager>
bool SkipList<K, V, RecordManager>::validate() {
    std::unordered_set<long long> keys;
    Node<K, V>* currentNode = head;
    long long prev_key = (head->key) - 1;
    while (currentNode != tail) {
        long long key = currentNode->key;
        if (keys.find(key) != keys.end() || key <= prev_key) {
            return false;
        }
        keys.insert(key);
        currentNode = currentNode->next[0];
    }
    return true;
}

template <typename K, typename V, class RecordManager>
void SkipList<K, V, RecordManager>::initThread(const int tid) {
    if (init[tid]) return;
    else init[tid] = !init[tid];
    recordManager->initThread(tid);
}

template <typename K, typename V, class RecordManager>
void SkipList<K, V, RecordManager>::deinitThread(const int tid) {
    if (!init[tid]) return;
    else init[tid] = !init[tid];
    recordManager->deinitThread(tid);
}

template <typename K, typename V, class RecordManager>
Node<K, V>* SkipList<K, V, RecordManager>::getRoot() {
    return head->next[0];
}

template <typename K, typename V, class RecordManager>
std::pair<int, V> SkipList<K, V, RecordManager>::find(const int tid, K key, Node<K, V>** preds, Node<K, V>** succs) {
    return find(tid, key, preds, succs, MAX_LEVEL);
}

template <typename K, typename V, class RecordManager>
std::pair<int, V> SkipList<K, V, RecordManager>::find(const int tid, K key, Node<K, V>** preds, Node<K, V>** succs, int maxLevel) {
    int flevel = -1;
    V val = NULL;
    Node<K, V>* pred = head;
    for (int level = maxLevel; level >= 0; level--) {
        Node<K, V>* curr = pred;
        while (key > curr->key) {
            pred = curr;
            curr = pred->next[level];
        }
        if (key == curr->key && flevel == -1) {
            flevel = level;
            val = curr->value;
        }
        preds[level] = pred;
        succs[level] = curr;
    }
    return std::make_pair(flevel, val);
}

template <typename K, typename V, class RecordManager>
SkipList<K, V, RecordManager>::~SkipList() {
    recordManager->printStatus();
    delete recordManager;
}

template <typename K, typename V, class RecordManager>
long long SkipList<K, V, RecordManager>::getPathsLength(const int tid) {
    return sumLengths[tid].value;
}

template <typename K, typename V, class RecordManager>
int SkipList<K, V, RecordManager>::getHeight() {
    return realMaxLvl;
}


#endif //LOCK_BASED_SKIP_LIST_SKIP_LIST_H



