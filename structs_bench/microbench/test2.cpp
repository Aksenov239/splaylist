//
// Created by demon1999 on 22.11.19.
//
#include <bits/stdc++.h>
#include "globals_extern.h"
#include "random_fnv1a.h"
#include "plaf.h"
#include "binding.h"
#include "papi_util_impl.h"
#include "rq_provider.h"
#include "keygen.h"
#include "adapter.h"

#define DS_ADAPTER_T ds_adapter<KEY_TYPE, VALUE_TYPE, RECLAIM<>, ALLOC<>, POOL<> >

const int MAX_KEY = 1000;
const int STRESS_MAX_THREADS = 10;

#define VALUE_TYPE void *
#define KEY_TYPE long long
#define KEY_TO_VALUE(key) &key
RandomFNV1A rngs[MAX_THREADS_POW2];
using namespace std;

void * NO_VALUE = NULL;

DS_ADAPTER_T* skipList = new DS_ADAPTER_T(STRESS_MAX_THREADS, 0, MAX_KEY, NO_VALUE, rngs);

const int OPERATIONS = 1000000;
const int THREAD_KEYS = 10000;
vector<vector<pair<int, long long> > > operations;
atomic<int> skipListSize(0);

void task(int j) {
    size_t pos = 0;
    for (pos = 0; pos < operations[j].size(); pos++) {
        long long key = operations[j][pos].second;
        int op_type = operations[j][pos].first;
        if (op_type == 0) {
            bool value = skipList->contains(j, key);
        }
        if (op_type == 1) {
            void* value = skipList->erase(j, key);
            if (value != NO_VALUE) {
                skipListSize--;
            }
        }
        if (op_type == 2) {
            void* value = skipList->insertIfAbsent(j, key, KEY_TO_VALUE(key));
            if (value == NULL) {
                skipListSize++;
            }
        }
    }
}

int realSize() {
    return skipList->getSize();
}

int main() {
    //srand(time(0));
    int numThreads = rand() % STRESS_MAX_THREADS + 1;
    operations.resize(numThreads, vector<pair<int, long long> >(0));
    for (int i = 0; i < OPERATIONS; i++) {
        int type = rand() % 3;
        int th = rand() % numThreads;
        int op_type = rand() % 3;
        long long key = rand() % (MAX_KEY - 1) + 1;
        operations[th].push_back(make_pair(op_type, key));
    }

    thread a[numThreads];
    for (int i = 0; i < numThreads; i++) {
        a[i] = thread(task, i);
    }
    for (int i = 0; i < numThreads; i++) {
        a[i].join();
    }
    cout << "Expected size is " << skipListSize << "\n";
    cout << "Real size is " << realSize() << "\n";
    cout << "Result of structure validation is " << (skipList->validateStructure()) << "\n";
    cout << "Number of threads is " << numThreads << "\n";
    if (skipListSize == realSize() && skipList->validateStructure()) {
        cout << "You are great!!!\n";
    } else {
        cout << "Something went wrong!!!\n";
        cout << "Expected size is " << skipListSize << "\n";
        cout << "Real size is " << realSize() << "\n";
        cout << "Result of structure validation is " << (skipList->validateStructure()) << "\n";
        cout << "Number of threads is " << numThreads << "\n";
        for (size_t i = 0; i < operations.size(); i++) {
            cout << "Operations on thread " << i << "\n";
            for (auto op : operations[i]) {
                if (op.first == 0)
                    cout << "contains ";
                if (op.first == 1)
                    cout << "erase ";
                if (op.first == 2)
                    cout << "insertIfAbsent ";
                cout << op.second << "\n";
            }
        }
    }
}

