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

const int OPERATIONS = 1000;
const int THREAD_KEYS = 10000;
const int PREFILL = 100;

vector<vector<pair<int, long long> > > operations;
vector<vector<pair<int, long long> > > opss;
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
            //skipList->printSummary();
            if (!value) {
                my_lock.lock();
                is_ok = false;
                my_lock.unlock();
            }
        } else {
          skipList->insertIfAbsent(j, key, KEY_TO_VALUE(key));
        }
    }
}
void task2(int j) {
   size_t pos = 0;
   for (pos = 0; pos < opss[j].size(); pos++) {
      long long key = opss[j][pos].second;
      int op_type = opss[j][pos].first;
      if (op_type == 1) {
         bool value = skipList->insertIfAbsent(j, key, KEY_TO_VALUE(key));
      }
   }
}

const double MAX_RAND = 30000 * 30000 - 1;
int genrand() {
    return (rand() % 30000) * 30000 + rand() % 30000;
}

const int NUM_THREADS = 4;
const double PC_Q = 0.99, PC_N = 0.01;

int main(int argc, char **argv) {
    int N;
    cin >> N;
   thread b[NUM_THREADS];
   opss.resize(NUM_THREADS, vector<pair<int, long long> >(0));
    for (int i = 0; i < N; i++) {
        int thr, key;
        cin >> thr >> key;
        s.insert(key);
        //skipList->insertIfAbsent(0, key, KEY_TO_VALUE(key));
       opss[genrand() % NUM_THREADS].push_back(std::make_pair(1, key));
       //skipList->insertIfAbsent(0, k, KEY_TO_VALUE(k));
    }
   for (int i = 0; i < NUM_THREADS; i++) {
      b[i] = thread(task2, i);
   }
   for (int i = 0; i < NUM_THREADS; i++) {
      b[i].join();
   }
    skipList->validateStructure();
//    exit(0);
    int M;
    cin >> M;
    operations.resize(NUM_THREADS, vector<pair<int, long long> >(0));
    for (int i = 0; i < M; i++) {
        int tp;
        int th;
        int key = 0;
        cin >> tp >> th >> key;
        operations[th].push_back(make_pair(tp, key));
    }
    thread a[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        a[i] = thread(task, i);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        a[i].join();
    }

    skipList->validateStructure();

//    for (size_t i = 0; i < operations.size(); i++) {
//        cout << "Operations on thread " << i << "\n";
//        int k = 0;
//        for (auto op : operations[i]) {
//            k++;
//            if (k > 10) break;
//            if (op.first == 0)
//                cout << "contains ";
//            if (op.first == 1)
//                cout << "erase ";
//            if (op.first == 2)
//                cout << "insertIfAbsent ";
//            cout << op.second << "\n";
//        }
//    }
//    cout << "Path sum size: " << (skipList->getSize()) << "\n";
//    if (is_ok) {
//        cout << "You are great!!!\n";
//    } else {
//        cout << "Something went wrong!!!\n";
//        cout << "Number of threads is " << NUM_THREADS << "\n";
//        for (size_t i = 0; i < operations.size(); i++) {
//            cout << "Operations on thread " << i << "\n";
//            for (auto op : operations[i]) {
//                if (op.first == 0)
//                    cout << "contains ";
//                cout << op.second << "\n";
//            }
//        }
//    }
}

