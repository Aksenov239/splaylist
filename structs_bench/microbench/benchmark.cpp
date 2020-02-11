//
// Created by demon1999 on 07.02.20.
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

const int MAX_KEY = 20000000;
const int STRESS_MAX_THREADS = 100;
typedef long long ll;

#define VALUE_TYPE void *
#define KEY_TYPE long long
#define KEY_TO_VALUE(key) &key
RandomFNV1A rngs[MAX_THREADS_POW2];
using namespace std;

void * NO_VALUE = NULL;

DS_ADAPTER_T* database = new DS_ADAPTER_T(STRESS_MAX_THREADS, 0, MAX_KEY, NO_VALUE, rngs);


int num_threads = 1, x = 100, y = 100, seconds = 1, presecs = 1, prefill = 10, cops = 1;

long long len[STRESS_MAX_THREADS + 1];
long long ops[STRESS_MAX_THREADS + 1];
long long sum_lengths, sum_ops;
vector<int> important, rest;

class MyPaddedRandom {
    volatile char pad0[PADDING_SIZE - sizeof(unsigned int)];
    unsigned int seed;
public:
    void setSeed(unsigned int _seed) {
        seed = _seed;
    }
    unsigned int genRandom() {
        seed ^= (seed << 6);
        seed ^= (seed >> 21);
        seed ^= (seed << 13);
        return seed;
    }
};

MyPaddedRandom rnd[STRESS_MAX_THREADS + 1];


void task(int tid) {
    //database->printSummary();
    auto beg = std::chrono::steady_clock::now();
    bool predperiod = true;
    for (long long i = 0; ; i++) {
        int k = -1;
        if (ll(rnd[tid].genRandom()) % 100 < x) {
            k = important[ll(rnd[tid].genRandom()) % int(important.size())];
        } else {
            k = rest[ll(rnd[tid].genRandom()) % int(rest.size())];
        }
        //std::cout << k << "\n";
        //database->printSummary();
        database->contains(tid, k);
        //std::cout << (database->getPathsLength(tid)) << " length\n";
        auto cur = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = cur - beg;
        if (diff.count() > double(presecs) - (1e-9) && predperiod) {
            predperiod = false;
            ops[tid] = -i;
            len[tid] -= database->getPathsLength(tid);
        }
        if (diff.count() > double(seconds + presecs) - (1e-9)) {
            ops[tid] += i;
            break;
        }
    }
    len[tid] += database->getPathsLength(tid);
    //__sync_fetch_and_add(&sum_lengths, database->getPathsLength(tid));
}

bool ideal = false;

int main(int argc, char **argv) {
    for (int i = 0; i <= STRESS_MAX_THREADS; i++) {
        rnd[i].setSeed(i + 1);
    }

    for (int i = 1; i < argc; i += 2) {
        //std::cout << argv[i] << "\n";
        if (string(argv[i]) == "x") {
            if (argc == i + 1) {
                std::cout << "Wrong arguments";
                exit(0);
            }
            x = atoi(argv[i + 1]);
        } else if (string(argv[i]) == "y") {
            y = atoi(argv[i + 1]);
        } else if (string(argv[i]) == "threads") {
            num_threads = atoi(argv[i + 1]);
        } else if (string(argv[i]) == "secs") {
            seconds = atoi(argv[i + 1]);
        } else if (string(argv[i]) == "presecs") {
            presecs = atoi(argv[i + 1]);
        } else if (string(argv[i]) == "prefill") {
            prefill = atoi(argv[i + 1]);
        } else if (string(argv[i]) == "cops") {
            cops = atoi(argv[i + 1]);
        } else if (string(argv[i]) == "ideal") {
            ideal = true;
        } else {
            std::cout << "Wrong arguments";
            exit(0);
        }

    }

    database->setCops(0, cops);

    set<int> keys;

    for (int i = 0; i < prefill; i++) {
        int key = ll(rnd[0].genRandom()) % MAX_KEY;
        while (key == 0 || keys.find(key) != keys.end()) {
            key = ll(rnd[0].genRandom()) % MAX_KEY;
        }
        if (!ideal)
            database->insertIfAbsent(0, key, KEY_TO_VALUE(key));
        keys.insert(key);
        if (ll(rnd[0].genRandom()) % 100 < y) {
            important.push_back(key);
        } else {
            rest.push_back(key);
        }
    }
    if (ideal) {
        for (auto key : keys) {
            database->insertIfAbsent(0, key, KEY_TO_VALUE(key));
        }
    }

    vector<thread> a(num_threads);

    for (int i = 0; i < num_threads; i++) {
        a[i] = thread(task, i + 1);
    }

    for (int i = 0; i < num_threads; i++) {
        a[i].join();
    }
    for (int i = 0; i < num_threads; i++) {
        sum_lengths += len[i + 1];
        sum_ops += ops[i + 1];
    }
    cout.precision(10);
    std::cout << "parameters:\n";
    std::cout << "threads: " << num_threads << "\n";
    std::cout << "x: " << x << "\n";
    std::cout << "y: " << y << "\n";
    std::cout << "prefill: " << prefill << "\n";
    std::cout << "secs: " << seconds << "\n";
    std::cout << "upd ops: " << cops << "\n";
    std::cout << "ideal: " << ideal << "\n";
    std::cout << "results:\n";
    std::cout << "h: " << (database->getHeight()) << ", ops: " << sum_ops << ", sumLengths: " << double(sum_lengths) / double(sum_ops) << "\n\n";
}

