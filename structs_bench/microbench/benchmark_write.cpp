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
const int MAX = 100000000;
typedef long long ll;

#define VALUE_TYPE void *
#define KEY_TYPE long long
#define KEY_TO_VALUE(key) &key
RandomFNV1A rngs[MAX_THREADS_POW2];
using namespace std;

void * NO_VALUE = NULL;

DS_ADAPTER_T* database = new DS_ADAPTER_T(STRESS_MAX_THREADS, 0, MAX_KEY, NO_VALUE, rngs);


int num_threads = 1, x = 100, y = 100, seconds = 1, presecs = 1, prefill = 10, cops = 1;
double pw = 0, wset = 0;

long long len[STRESS_MAX_THREADS + 1];
long long ops[STRESS_MAX_THREADS + 1];
long long sum_lengths, sum_ops;
vector<int> important, rest;
vector<int> working_keys;
set<int> test_set;

std::atomic<bool> finish(false);
int slack = 1000;

class MyPaddedRandom {
    volatile char pad0[128 - sizeof(unsigned int)];
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

class MyKeyGeneratorZipf {
public:
    PAD;
    int maxKey;
    double alpha;
    double c = 0; // Normalization constant
    double * sum_probs; // Pre-calculated sum of probabilities
    PAD;
    
    MyKeyGeneratorZipf(int _maxKey, double _alpha) {
//        std::cout<<"start MyKeyGeneratorZipfData"<<std::endl;
        maxKey = _maxKey;
        alpha = _alpha;
        
        // Compute normalization constant c for implied key range: [1, maxKey]
        int i;
        for (i = 1; i <= _maxKey; i++)
            c = c + (1.0 / pow((double) i, alpha));
        c = 1.0 / c;

        sum_probs = new double[_maxKey+1];
        sum_probs[0] = 0;
        for (i = 1; i <= _maxKey; i++) {
            sum_probs[i] = sum_probs[i - 1] + c / pow((double) i, alpha);
        }
//        std::cout<<"end MyKeyGeneratorZipfData"<<std::endl;
    }

    int generateValue(int tid) {
        double z; // Uniform random number (0 < z < 1)
        int zipf_value; // Computed exponential value to be returned
        int low, high, mid; // Binary-search bounds

        // Pull a uniform random number (0 < z < 1)
        do {
            z = (rnd[tid].genRandom() / (double) std::numeric_limits<unsigned int>::max());
            assert((0 <= z) && (z <= 1));
//            printf("    z=%lf\n", z);
        } while ((z == 0) || (z == 1));

        // Map z to the value
        low = 1, high = maxKey, mid;
        do {
            mid = floor((low + high) / 2);
            if (sum_probs[mid] >= z && sum_probs[mid - 1] < z) {
                zipf_value = mid;
                break;
            } else if (sum_probs[mid] >= z) {
                high = mid - 1;
            } else {
                low = mid + 1;
            }
        } while (low <= high);

        // Assert that zipf_value is between 1 and N
        assert((zipf_value >= 1) && (zipf_value <= maxKey));

        return (zipf_value);
    }

    ~MyKeyGeneratorZipf() {
        delete[] sum_probs;
    }
};

MyKeyGeneratorZipf* myKeyGeneratorZipf;
bool zipf = false;

void task(int tid) {
//    std::cerr << tid << " on CPU " << sched_getcpu() << std::endl;
    //database->printSummary();
    auto beg = std::chrono::steady_clock::now();
    bool predperiod = true;
    int inserts = 0;
    for (long long i = 0; ; i++) {
        int k = -1;
        if (ll(rnd[tid].genRandom()) % MAX < MAX * pw) {
            k = working_keys[ll(rnd[tid].genRandom()) % int(working_keys.size())];
            if (ll(rnd[tid].genRandom()) % 2 == 0) {
                VALUE_TYPE v = database->insertIfAbsent(tid, k, KEY_TO_VALUE(k));
            } else {
                VALUE_TYPE v = database->erase(tid, k);
            }
//            test_set.insert(k);
        } else {
            if (zipf) {
    //            std::cerr << "Generated: " << myKeyGeneratorZipf->generateValue(tid) << std::endl;
    //            std::cerr << important.size() << std::endl;
                k = important[myKeyGeneratorZipf->generateValue(tid) - 1];
            } else {
                if (ll(rnd[tid].genRandom()) % 100 < x) {
                    k = important[ll(rnd[tid].genRandom()) % int(important.size())];
                } else {
                    k = rest[ll(rnd[tid].genRandom()) % int(rest.size())];
                }
            }
            //std::cout << k << "\n";
            //database->printSummary();
            database->contains(tid, k);
        }
        //std::cout << (database->getPathsLength(tid)) << " length\n";
        if (i % 1000 == 0) {
          auto cur = std::chrono::steady_clock::now();
          std::chrono::duration<double> diff = cur - beg;
          if (diff.count() > double(presecs) - (1e-9) && predperiod) {
              database->warmupEnd();
              predperiod = false;
              ops[tid] = -i;
              len[tid] -= database->getPathsLength(tid);
          }
          if (diff.count() > double(seconds + presecs) - (1e-9)) {
              ops[tid] += i;
              break;
          }
        }
    }
    len[tid] += database->getPathsLength(tid);
}

bool ideal = false;

int main(int argc, char **argv) {
    for (int i = 0; i <= STRESS_MAX_THREADS; i++) {
        rnd[i].setSeed(i + 1);
    }

    zipf = false;
    double alpha = 0;
    pw = 0;

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
        } else if (string(argv[i]) == "zipf") {
            zipf = true;
            alpha = atof(argv[i + 1]);
        } else if (string(argv[i]) == "write") {
            pw = atof(argv[i + 1]);        
        } else if (string(argv[i]) == "writeset") {
            wset = atof(argv[i + 1]);
        } else {
            std::cout << "Wrong arguments";
            exit(0);
        }
    }

    myKeyGeneratorZipf = new MyKeyGeneratorZipf(prefill, alpha);

    database->setCops(0, cops);

    set<int> keys;

    for (int i = 0; i < prefill; i++) {

        int key = ll(rnd[0].genRandom()) % MAX_KEY;
        while (key == 0 || keys.find(key) != keys.end()) {
            key = ll(rnd[0].genRandom()) % MAX_KEY;
        }
        bool inserted = false;
        if (!ideal) {
            if (ll(rnd[0].genRandom()) % 2 == 0) {
                VALUE_TYPE v = database->insertIfAbsent(0, key, KEY_TO_VALUE(key));
                inserted = true;
//                test_set.insert(key);
            }
        }
        keys.insert(key);
        if (ll(rnd[0].genRandom() % MAX) < MAX * wset) {
            working_keys.push_back(key);
        }
        if (inserted) {
            if (ll(rnd[0].genRandom()) % 100 < 2 * y || zipf) {
                important.push_back(key);
            } else {
                rest.push_back(key);
            }
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

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);
        int rc = pthread_setaffinity_np(a[i].native_handle(),
                                        sizeof(cpu_set_t), &cpuset);
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
    std::cout << "write prob: " << pw << "\n";
    std::cout << "write set: " << wset << "\n";
    std::cout << "zipf: " << zipf << "\n";
    std::cout << "alpha: " << alpha << "\n";
    std::cout << "x: " << x << "\n";
    std::cout << "y: " << y << "\n";
    std::cout << "prefill: " << prefill << "\n";
    std::cout << "secs: " << seconds << "\n";
    std::cout << "upd ops: " << cops << "\n";
    std::cout << "ideal: " << ideal << "\n";
    std::cout << "results:\n";
    std::cout << "h: " << 0 /*(database->getHeight())*/ << ", ops: " << sum_ops << ", sumLengths: " << double(sum_lengths) / double(sum_ops) << "\n\n";
}
