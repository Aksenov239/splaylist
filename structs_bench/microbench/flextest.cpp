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

const int MAX_KEY = 2000000;
const int STRESS_MAX_THREADS = 10;

#define VALUE_TYPE void *
#define KEY_TYPE long long
#define KEY_TO_VALUE(key) &key
RandomFNV1A rngs[MAX_THREADS_POW2];
using namespace std;

void * NO_VALUE = NULL;

DS_ADAPTER_T* skipList = new DS_ADAPTER_T(STRESS_MAX_THREADS, 0, MAX_KEY, NO_VALUE, rngs);

const int OPERATIONS = 10000000;
const int PREFILL = 1000000;

vector<vector<pair<int, long long> > > operations;

mutex my_lock;
bool is_ok = true;

set<long long> s;
void task(int j) {
    size_t pos = 0;
    for (pos = 0; pos < operations[j].size(); pos++) {
        long long key = operations[j][pos].second;
        int op_type = operations[j][pos].first;
        if (op_type == 0) {
            bool value = skipList->contains(j, key);
            if (!value) {
                my_lock.lock();
                is_ok = false;
                my_lock.unlock();
            }
        }
    }
}

const double MAX_RAND = 30000 * 30000 - 1;
int genrand() {
    return (rand() % 30000) * 30000 + rand() % 30000;
}

const int NUM_THREADS = 1;
const double PC_Q = 0.99, PC_N = 0.01;

int main(int argc, char **argv) {
    srand(0);
    //cout << atoi(argv[1]) << "\n";
    set<long long> ks;
    ks.insert(0);
    vector<long long> keys;
    long long min_key = 0;
    long long max_key = MAX_KEY;
    for (int i = 0; i < PREFILL; i++) {
        long long k = genrand() % MAX_KEY;
        while (ks.find(k) != ks.end()) {
            k = genrand() % MAX_KEY;
        }
        keys.push_back(k);
        ks.insert(k);
        s.insert(k);
        skipList->insertIfAbsent(0, k, KEY_TO_VALUE(k));
    }
    //exit(0);
    for (int i = 0; i < 100; i++) {
        int a = genrand() % int(keys.size());
        int b = genrand() % int(keys.size());
        swap(keys[a], keys[b]);
    }
    //random_shuffle(keys.begin(), keys.end());
    operations.resize(NUM_THREADS, vector<pair<int, long long> >(0));
    int high_size = (1 + int(keys.size() * PC_N));
    for (int i = 0; i < OPERATIONS; i++) {
        int op_type = 0;
        int th = genrand() % NUM_THREADS;
        int key = 0;
        if (genrand() < MAX_RAND * PC_Q) {
            key = keys[genrand() % high_size];
        } else {
            key = keys[genrand() % (int(keys.size()) - high_size) + high_size];
        }
        operations[th].push_back(make_pair(op_type, key));
    }
    thread a[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        a[i] = thread(task, i);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        a[i].join();
    }

    for (size_t i = 0; i < operations.size(); i++) {
        cout << "Operations on thread " << i << "\n";
        int k = 0;
        for (auto op : operations[i]) {
            k++;
            if (k > 10) break;
            if (op.first == 0)
                cout << "contains ";
            if (op.first == 1)
                cout << "erase ";
            if (op.first == 2)
                cout << "insertIfAbsent ";
            cout << op.second << "\n";
        }
    }
    cout << "Path sum size: " << (skipList->getSize()) << "\n";
    if (is_ok) {
        cout << "You are great!!!\n";
    } else {
        cout << "Something went wrong!!!\n";
        cout << "Number of threads is " << NUM_THREADS << "\n";
        for (size_t i = 0; i < operations.size(); i++) {
            cout << "Operations on thread " << i << "\n";
            for (auto op : operations[i]) {
                if (op.first == 0)
                    cout << "contains ";
                cout << op.second << "\n";
            }
        }
    }
}

