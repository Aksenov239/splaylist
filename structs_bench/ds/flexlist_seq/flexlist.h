//
// Created by demon1999 on 15.11.19.
//

#ifndef SEQ_FLEX_LIST_FLEX_LIST_H
#define SEQ_FLEX_LIST_FLEX_LIST_H
const int MAX_LEVEL = 30;
const int MAX_THREADS = 110;
const int PADDING_SIZE = 128;

template<typename K, typename V>
struct Node {
    K key;
    V value;
    int zeroLevel;
    int topLevel;
    int selfhits;
    Node<K, V>* next[MAX_LEVEL + 1];
    int hits[MAX_LEVEL + 1];
};


template<typename K, typename V, class RecordManager>
class FlexList {
private:
    int UPDATE_NUM = 1;
    long long sumLengths;
    int zeroLevel = MAX_LEVEL - 1;
    long long update_counter;
    Node<K, V>* head;
    Node<K, V>* tail;
    RecordManager * const recordManager;
    V noValue;
    int init[MAX_THREADS] = {0,};
    long long sum_lengths = 0;
    long long accessCounter = 0;
public:
    FlexList(const int numThreads, const V noValue, const K minKey, const K maxKey);

    ~FlexList();

    Node<K, V>* createNode(const int tid, int h, K key, V value);

    V qfind(const int tid, const K &key);

    bool contains(const int tid, const K &key);

    V insertIfAbsent(const int tid, const K &key, const V &value);

    bool validate();

    void setCops(int cops);

    void printDebuggingDetails();

    void initThread(const int tid);

    void deinitThread(const int tid);

    long long getPathsLength(const int tid);
    int getHeight();

    Node<K, V>* getRoot();

private:
    bool find(const int tid, K key, Node<K, V>*& pred, Node<K, V>*& succ);
    void update(const int tid, K key);
    void updateZeroLevel(Node<K, V> *curr);//This function supposes that curr is locked now
    int getHits(Node<K, V> *curr, int h);
};

template <typename K, typename V, class RecordManager>
void FlexList<K, V, RecordManager>::updateZeroLevel(Node<K, V> *curr) {
    // This function should be used only if we have lock on curr
    int currZeroLevel = zeroLevel;
    if ((curr->zeroLevel) > currZeroLevel) {
        //assert(cur->zeroLevel > 0);
        curr->hits[(curr->zeroLevel) - 1] = 0;
        curr->next[(curr->zeroLevel) - 1] = curr->next[(curr->zeroLevel)];
        (curr->zeroLevel)--;
    }
}

template <typename K, typename V, class RecordManager>
void FlexList<K, V, RecordManager>::update(const int tid, K key) {
    accessCounter++;
    int curAccess = accessCounter;
    head->hits[MAX_LEVEL]++;
    Node<K, V>* pred = head;
    for (int h = MAX_LEVEL - 1; h >= zeroLevel; h--) {
        while (pred->zeroLevel > h) {
            updateZeroLevel(pred);
        }

        Node<K, V>* predpred = pred;
        Node<K, V>* cur = pred->next[h];

        if (cur->zeroLevel > h) {
            while (cur->zeroLevel > h) {
                updateZeroLevel(cur);
            }
        }
        sum_lengths++;
        if ((cur->key) > key) {
            (pred->hits[h])++;
            continue;
        }
        bool ok = false;
        while ((cur->key) <= key) {
            if (cur->zeroLevel > h) {
                while (cur->zeroLevel > h) {
                    updateZeroLevel(cur);
                }
            }
            sum_lengths++;
            if((cur->next[h]->key) > key) {
                if (cur->key == key) {
                    cur->selfhits++;
                    ok = true;
                } else
                    cur->hits[h]++;
            }
            if (h + 1 < MAX_LEVEL && h < (predpred->topLevel) && (predpred->next[h]) == cur && ((predpred->hits[h + 1]) - (predpred->hits[h])) > (curAccess / (1 << (MAX_LEVEL - 1 - h - 1)))) {
                int curh = (cur->topLevel);
                while (curh + 1 < MAX_LEVEL && curh < (predpred->topLevel) &&
                       ((predpred->hits[curh + 1]) - (predpred->hits[curh])) > (curAccess / (1 << (MAX_LEVEL - 1 - curh - 1)))) {
                    (cur->topLevel++);
                    curh++;
                    cur->hits[curh] = ((predpred->hits[curh]) - (predpred->hits[curh - 1]) - (cur->selfhits));
                    cur->next[curh] = predpred->next[curh];
                    predpred->next[curh] = cur;
                    predpred->hits[(curh)] = (predpred->hits[(curh) - 1]);
                }
                predpred = cur;
                pred = cur;
                cur = cur->next[h];
                continue;
            } else if ((cur->topLevel) == h && (cur->next[h]->key) <= key && getHits(cur, h) + getHits(pred, h) <= (curAccess / (1 << (MAX_LEVEL - 1 - h)))) {
                int curZeroLevel = zeroLevel;
                if (h == curZeroLevel) {
                    zeroLevel = curZeroLevel - 1;
                }
                //assert((cur->topLevel) == h);
                if ((cur->zeroLevel) > h - 1) {
                    updateZeroLevel(cur);
                }
                if ((pred->zeroLevel) > h - 1) {
                    updateZeroLevel(pred);
                }
                int cur_hits = getHits(cur, h);
                pred->hits[h] += cur_hits;
                cur->hits[h] = 0;
                pred->next[h] = cur->next[h];
                cur->next[h] = NULL;
                if (pred != predpred) {
                }
                cur->topLevel--;
                cur = pred->next[h];
                continue;
            }
            pred = cur;
            cur = cur->next[h];
        }
        if (ok) {
            return;
        }
    }
}

template <typename K, typename V, class RecordManager>
int FlexList<K, V, RecordManager>::getHits(Node<K, V> *curr, int h) {
    if (curr->zeroLevel > h)
        return curr->selfhits;
    return curr->selfhits + curr->hits[h];
}


template <typename K, typename V, class RecordManager>
Node<K, V>* FlexList<K, V, RecordManager>::createNode(const int tid, int h, K key, V value) {
    Node<K, V>* node = recordManager->template allocate<Node<K,V>>(tid);//new Node<K, V>();//
    node->topLevel = h;
    node->key = key;
    node->value = value;
    node->zeroLevel = h;
    node->selfhits = 0;

    for (int i = 0; i <= MAX_LEVEL; i++) {
        node->hits[i] = 0;
        node->next[i] = NULL;
    }
    return node;
}

template <typename K, typename V, class RecordManager>
FlexList<K, V, RecordManager>::FlexList(const int numThreads, const V noValue, const K minKey, const K maxKey) :
    recordManager(new RecordManager(numThreads)) {
    const int tid = 0;
    initThread(tid);
    recordManager->endOp(tid);
    head = createNode(tid, zeroLevel, minKey, 0);
    tail = createNode(tid, zeroLevel, maxKey, 0);
    head->topLevel = MAX_LEVEL;
    tail->topLevel = MAX_LEVEL;
    this->noValue = noValue;
    for (unsigned int i = 0; i <= numThreads; i++) {
        sumLengths = 0;
        update_counter = 0;
    }
    for (int i = zeroLevel; i <= MAX_LEVEL; i++) {
        head->next[i] = tail;
        tail->next[i] = NULL;
    }
}

template <typename K, typename V, class RecordManager>
bool FlexList<K, V, RecordManager>::contains(const int tid, const K &key) {
    Node<K, V>* pred;
    Node<K, V>* succ;
    if (find(tid, key, pred, succ)) {
        update_counter++;
        if (update_counter == UPDATE_NUM) {
            update(tid, key);
            update_counter = 0;
        }
        return true;
    }
    return false;
}

template <typename K, typename V, class RecordManager>
V FlexList<K, V, RecordManager>::qfind(const int tid, const K &key) {
    Node<K, V>* pred;
    Node<K, V>* succ;
    if (find(tid, key, pred, succ)) {
        V v = (succ->value);
        update_counter++;
        if (update_counter == UPDATE_NUM) {
            update(tid, key);
            update_counter = 0;
        }
        return v;
    }
    return noValue;
}

template <typename K, typename V, class RecordManager>
V FlexList<K, V, RecordManager>::insertIfAbsent(const int tid, const K &key, const V &value) {
    while (true) {
        Node<K, V> *pred = NULL;
        Node<K, V> *succ = NULL;
        if (find(tid, key, pred, succ)) {
            return succ->value;
        }
        int curZeroLevel = (pred->zeroLevel);
        Node<K, V>* newNode = createNode(tid, curZeroLevel, key, value);
        newNode->next[curZeroLevel] = succ;

        if (pred->next[curZeroLevel] == succ) {
            pred->next[curZeroLevel] = newNode;
            update(tid, key);
            return noValue;
        }
        recordManager->retire(tid, newNode);
    }
}

template <typename K, typename V, class RecordManager>
bool FlexList<K, V, RecordManager>::validate() {
    return true;
}

template <typename K, typename V, class RecordManager>
void FlexList<K, V, RecordManager>::initThread(const int tid) {
    if (init[tid]) return;
    else init[tid] = !init[tid];
    recordManager->initThread(tid);
}

template <typename K, typename V, class RecordManager>
void FlexList<K, V, RecordManager>::setCops(int cops) {
    UPDATE_NUM = cops;
}

template <typename K, typename V, class RecordManager>
void FlexList<K, V, RecordManager>::printDebuggingDetails() {
    for (int h = zeroLevel; h <= MAX_LEVEL; h++) {
        Node<K, V>* cur = head;
        std::cout << h << ": ";
        while (cur != tail) {
            std::cout << (cur->key) << " ";
            Node<K, V>* cnext = NULL;
            if ((cur->zeroLevel) > h) {
                cnext = cur->next[cur->zeroLevel];
            } else
                cnext = (cur->next[h]);
            if ((cur->key) >= (cnext->key))
                exit(0);
            cur = cnext;
        }
        std::cout << "\n";
    }
    return;
}

template <typename K, typename V, class RecordManager>
void FlexList<K, V, RecordManager>::deinitThread(const int tid) {
    if (!init[tid]) return;
    else init[tid] = !init[tid];
    recordManager->deinitThread(tid);
}

template <typename K, typename V, class RecordManager>
Node<K, V>* FlexList<K, V, RecordManager>::getRoot() {
    return head->next[0];
}

template <typename K, typename V, class RecordManager>
bool FlexList<K, V, RecordManager>::find(const int tid, K key, Node<K, V>*& pred, Node<K, V>*& succ) {
    //std::cout <<"find\n";
    pred = head;
    succ = (head->next[MAX_LEVEL]);
    for (int level = MAX_LEVEL - 1; level >= zeroLevel; level--) {
        sumLengths++;
        if (pred->zeroLevel > level) {
            while ((pred->zeroLevel) > level)
                updateZeroLevel(pred);
        }

        succ = (pred->next[level]);

        if (succ->zeroLevel > level) {
            while ((succ->zeroLevel) > level)
                updateZeroLevel(succ);
        }

        while (key > succ->key) {
            sumLengths++;
            pred = succ;
            succ = pred->next[level];
            if (succ->zeroLevel > level) {
                while ((succ->zeroLevel) > level)
                    updateZeroLevel(succ);
            }
        }
        if (key == succ->key) {
            return true;
        }
    }
    return false;
}

template <typename K, typename V, class RecordManager>
FlexList<K, V, RecordManager>::~FlexList() {
    recordManager->printStatus();
    delete recordManager;
}

template <typename K, typename V, class RecordManager>
long long FlexList<K, V, RecordManager>::getPathsLength(const int tid) {
    return sumLengths;
}

template <typename K, typename V, class RecordManager>
int FlexList<K, V, RecordManager>::getHeight() {
    return MAX_LEVEL - 1 - zeroLevel;
}

#endif //SEQ_FLEX_LIST_FLEX_LIST_H




