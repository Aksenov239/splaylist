//
// Created by demon1999 on 22.11.19.
//
#include <bits/stdc++.h>

const int MAX_KEY = 10000;

using namespace std;

const int OPERATIONS = 100;
const int THREAD_KEYS = 10000;
const int PREFILL = 30;

vector<vector<pair<int, long long> > > operations;

const double MAX_RAND = 30000 * 30000 - 1;
int genrand() {
    return (rand() % 30000) * 30000 + rand() % 30000;
}

const int NUM_THREADS = 2;
const double PC_Q = 0.01, PC_N = 0.5;

int main(int argc, char **argv) {
    srand(atoi(argv[1]));
    set<long long> ks;
    ks.insert(0);
    vector<long long> keys;
    long long min_key = 0;
    long long max_key = MAX_KEY;
    cout << PREFILL << "\n";
    for (int i = 0; i < PREFILL; i++) {
        long long k = genrand() % MAX_KEY;
        while (ks.find(k) != ks.end()) {
            k = genrand() % MAX_KEY;
        }
        keys.push_back(k);
        ks.insert(k);
        cout << genrand() % NUM_THREADS << " " << k << "\n";
    }
    random_shuffle(keys.begin(), keys.end());
    operations.resize(NUM_THREADS, vector<pair<int, long long> >(0));
    int high_size = (1 + int(keys.size() * PC_N));
    cout << OPERATIONS << "\n";
    for (int i = 0; i < OPERATIONS; i++) {
        int op_type = genrand() % 2;
        int th = genrand() % NUM_THREADS;
        int key = 0;
//        if (genrand() < MAX_RAND * PC_Q) {
//            key = keys[genrand() % high_size];
//        } else {
//            key = keys[genrand() % (int(keys.size()) - high_size) + high_size];
//        }
        if (op_type == 1)
            key = genrand() % MAX_KEY;
        else
            key = keys[genrand() % int(keys.size())];
        if (key == 0)
            key = 1;
        cout << op_type << " " << th << " " << key << "\n";
        operations[th].push_back(make_pair(op_type, key));
    }
}

