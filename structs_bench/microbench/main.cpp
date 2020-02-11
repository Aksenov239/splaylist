/**
 * Setbench test harness for performing rigorous data structure microbenchmarks.
 *
 * Copyright (C) 2018 Trevor Brown
 */

#define MICROBENCH

typedef long long test_type;

#include <limits>
#include <cstring>
#include <ctime>
#include <thread>
#include <atomic>
#include <chrono>
#include <cassert>
#include <parallel/algorithm>
#include <omp.h>
#include <perftools.h>

#ifdef PRINT_JEMALLOC_STATS
#include <jemalloc/jemalloc.h>
    #define DEBUG_PRINT_ARENA_STATS malloc_stats_print(printCallback, NULL, "ag")
    void printCallback(void * nothing, const char * data) {
        std::cout<<data;
    }
#else
#define DEBUG_PRINT_ARENA_STATS
#endif

// configure global statistics tracking and output using GSTATS (common/gstats/)
#include "configure_gstats.h" // note: it is crucial that this is included before any other of the other headers below

// each thread saves its own thread-id (should be used primarily within this file--could be eliminated to improve software engineering)
__thread int tid = 0;

// some read-only globals (could be encapsulated in a struct and passed around to improve software engineering)

#include "plaf.h"
PAD;
double INS;
double DEL;
double RQ;
int RQSIZE;
int MAXKEY = 0;
int MILLIS_TO_RUN;
int DESIRED_PREFILL_SIZE;
bool PREFILL;
int PREFILL_THREADS;
int WORK_THREADS;
int RQ_THREADS;
int TOTAL_THREADS;
PAD;

#include "globals_extern.h"
#include "random_fnv1a.h"
#include "plaf.h"
#include "binding.h"
#include "papi_util_impl.h"
#include "rq_provider.h"
#include "keygen.h"

#ifndef PRINTS
#define STR(x) XSTR(x)
#define XSTR(x) #x
#define PRINTI(name) { std::cout<<#name<<"="<<name<<std::endl; }
#define PRINTS(name) { std::cout<<#name<<"="<<STR(name)<<std::endl; }
#endif

#include "adapter.h" /* data structure adapter header (selected according to the "ds/..." subdirectory in the -I include paths */
#include "tree_stats.h"
#define DS_ADAPTER_T ds_adapter<test_type, VALUE_TYPE, RECLAIM<>, ALLOC<>, POOL<> >

#ifndef INSERT_FUNC
#define INSERT_FUNC insertIfAbsent
#endif

#ifdef RQ_SNAPCOLLECTOR
#define RQ_SNAPCOLLECTOR_OBJECT_TYPES , SnapCollector<node_t<test_type, test_type>, test_type>, SnapCollector<node_t<test_type, test_type>, test_type>::NodeWrapper, ReportItem, CompactReportItem
    #define RQ_SNAPCOLLECTOR_OBJ_SIZES <<" SnapCollector="<<(sizeof(SnapCollector<node_t<test_type, test_type>, test_type>))<<" NodeWrapper="<<(sizeof(SnapCollector<node_t<test_type, test_type>, test_type>::NodeWrapper))<<" ReportItem="<<(sizeof(ReportItem))<<" CompactReportItem="<<(sizeof(CompactReportItem))
#else
#define RQ_SNAPCOLLECTOR_OBJECT_TYPES
#define RQ_SNAPCOLLECTOR_OBJ_SIZES
#endif

#define KEY_TO_VALUE(key) &key /* note: hack to turn a key into a pointer */
#define VALUE_TYPE void *

#ifdef USE_RCU
#include "eer_prcu_impl.h"
    #define __RCU_INIT_THREAD urcu::registerThread(tid);
    #define __RCU_DEINIT_THREAD urcu::unregisterThread();
    #define __RCU_INIT_ALL urcu::init(TOTAL_THREADS);
    #define __RCU_DEINIT_ALL urcu::deinit(TOTAL_THREADS);
#else
#define __RCU_INIT_THREAD
#define __RCU_DEINIT_THREAD
#define __RCU_INIT_ALL
#define __RCU_DEINIT_ALL
#endif

#ifdef USE_RLU
#include "rlu.h"
    PAD;
    __thread rlu_thread_data_t * rlu_self;
    PAD;
    rlu_thread_data_t * rlu_tdata = NULL;
    #define __RLU_INIT_THREAD rlu_self = &rlu_tdata[tid]; RLU_THREAD_INIT(rlu_self);
    #define __RLU_DEINIT_THREAD RLU_THREAD_FINISH(rlu_self);
    #define __RLU_INIT_ALL rlu_tdata = new rlu_thread_data_t[MAX_THREADS_POW2]; RLU_INIT(RLU_TYPE_FINE_GRAINED, 1);
    #define __RLU_DEINIT_ALL RLU_FINISH(); delete[] rlu_tdata;
#else
#define __RLU_INIT_THREAD
#define __RLU_DEINIT_THREAD
#define __RLU_INIT_ALL
#define __RLU_DEINIT_ALL
#endif

#define INIT_THREAD(tid) \
    __RLU_INIT_THREAD; \
    __RCU_INIT_THREAD; \
    g->dsAdapter->initThread(tid);
#define DEINIT_THREAD(tid) \
    g->dsAdapter->deinitThread(tid); \
    __RCU_DEINIT_THREAD; \
    __RLU_DEINIT_THREAD;
#define INIT_ALL \
    __RCU_INIT_ALL; \
    __RLU_INIT_ALL;
#define DEINIT_ALL \
    __RLU_DEINIT_ALL; \
    __RCU_DEINIT_ALL;





enum KeyGeneratorDistribution {
    UNIFORM, ZIPF
};

template <class KeyGenT>
struct globals_t {
    PAD;
    // const
    void * const NO_VALUE;
    const test_type KEY_MIN; // must be smaller than any key that can be inserted/deleted
    const test_type KEY_MAX; // must be less than std::max(), because the snap collector needs a reserved key larger than this! (and larger than any key that can be inserted/deleted)
    const long long PREFILL_INTERVAL_MILLIS;
    PAD;
    // write once
    long elapsedMillis;
    long long prefillKeySum;
    long long prefillSize;
    std::chrono::time_point<std::chrono::high_resolution_clock> programExecutionStartTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
    PAD;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    long long startClockTicks;
    PAD;
    long elapsedMillisNapping;
    volatile long long prefillIntervalElapsedMillis;
    std::chrono::time_point<std::chrono::high_resolution_clock> prefillStartTime;
    PAD;
    volatile test_type garbage; // used to prevent optimizing out some code
    PAD;
    DS_ADAPTER_T * dsAdapter; // the data structure
    PAD;
    KeyGeneratorZipfData * keygenZipfData;
    KeyGenT * keygens[MAX_THREADS_POW2];
    PAD;
    RandomFNV1A rngs[MAX_THREADS_POW2]; // create per-thread random number generators (padded to avoid false sharing)
//    PAD; // not needed because of padding at the end of rngs
    volatile bool start;
    PAD;
    volatile bool done;
    PAD;
    volatile int running; // number of threads that are running
    PAD;

    globals_t(size_t maxkeyToGenerate, KeyGeneratorDistribution distribution)
            : NO_VALUE(NULL)
            , KEY_MIN(0) /*std::numeric_limits<test_type>::min()+1)*/
            , KEY_MAX(std::numeric_limits<test_type>::max()-1)
            , PREFILL_INTERVAL_MILLIS(200)
    {
        keygenZipfData = NULL;
        srand(time(0));
        for (int i=0;i<MAX_THREADS_POW2;++i) {
            rngs[i].setSeed(rand());
        }
        switch (distribution) {
            case ZIPF: {
                double zipfTheta = 0.5;
                keygenZipfData = new KeyGeneratorZipfData(maxkeyToGenerate, zipfTheta);
                for (int i=0;i<MAX_THREADS_POW2;++i) {
                    keygens[i] = (KeyGenT *) new KeyGeneratorZipf<test_type>(keygenZipfData, &rngs[i]);
                }
            } break;
            case UNIFORM: {
                for (int i=0;i<MAX_THREADS_POW2;++i) {
                    keygens[i] = (KeyGenT *) new KeyGeneratorUniform<test_type>(&rngs[i], maxkeyToGenerate);
                }
            } break;
            default: {
                setbench_error("invalid case");
            } break;
        }

        start = false;
        done = false;
        running = 0;
        dsAdapter = NULL;
        garbage = 0;
        prefillIntervalElapsedMillis = 0;
        prefillKeySum = 0;
        prefillSize = 0;
    }
    ~globals_t() {
        for (int i=0;i<MAX_THREADS_POW2;++i) {
            if (keygens[i]) delete keygens[i];
        }
        if (keygenZipfData) delete keygenZipfData;
    }
};

#ifndef OPS_BETWEEN_TIME_CHECKS
#define OPS_BETWEEN_TIME_CHECKS 100
#endif
#ifndef RQS_BETWEEN_TIME_CHECKS
#define RQS_BETWEEN_TIME_CHECKS 10
#endif

template <class GlobalsT>
void thread_prefill_with_updates(GlobalsT * g, int __tid) {
    tid = __tid;
    binding_bindThread(tid);
    test_type garbage = 0;

    double insProbability = (INS > 0 ? 100*INS/(INS+DEL) : 50.);

    INIT_THREAD(tid);
    __sync_fetch_and_add(&g->running, 1);
    __sync_synchronize();
    while (!g->start) { __sync_synchronize(); TRACE COUTATOMICTID("waiting to start"<<std::endl); } // wait to start
    int cnt = 0;
    auto __endTime = g->startTime;
    while (!g->done) {
        if (((++cnt) % OPS_BETWEEN_TIME_CHECKS) == 0) {
            __endTime = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(__endTime-g->startTime).count() >= g->PREFILL_INTERVAL_MILLIS) {
                g->done = true;
                __sync_synchronize();
                break;
            }
        }

        VERBOSE if (cnt&&((cnt % 1000000) == 0)) COUTATOMICTID("op# "<<cnt<<std::endl);
        test_type key = g->keygens[tid]->next();
        //test_type key = g->rngs[tid].next(MAXKEY) + 1;
        double op = g->rngs[tid].next(100000000) / 1000000.;
        GSTATS_TIMER_RESET(tid, timer_latency);
        if (op < insProbability) {
            if (g->dsAdapter->INSERT_FUNC(tid, key, KEY_TO_VALUE(key)) == g->dsAdapter->getNoValue()) {
                GSTATS_ADD(tid, key_checksum, key);
                GSTATS_ADD(tid, prefill_size, 1);
            }
            GSTATS_ADD(tid, num_inserts, 1);
        } else {
            if (g->dsAdapter->erase(tid, key) != g->dsAdapter->getNoValue()) {
                GSTATS_ADD(tid, key_checksum, -key);
                GSTATS_ADD(tid, prefill_size, -1);
            }
            GSTATS_ADD(tid, num_deletes, 1);
        }
        GSTATS_ADD(tid, num_operations, 1);
        GSTATS_TIMER_APPEND_ELAPSED(tid, timer_latency, latency_updates);
    }

    __sync_fetch_and_add(&g->running, -1);
    while (g->running) {
        // wait
    }

    DEINIT_THREAD(tid);
    g->garbage += garbage;
    __sync_bool_compare_and_swap(&g->prefillIntervalElapsedMillis, 0, std::chrono::duration_cast<std::chrono::milliseconds>(__endTime-g->startTime).count());
}

// note: this function guarantees that exactly expectedSize keys are inserted into the data structure by the end
void prefillWithInserts(auto g, int64_t expectedSize) {
    std::cout<<"Info: prefilling using INSERTION ONLY."<<std::endl;
    auto prefillStartTime = std::chrono::high_resolution_clock::now();

    const int tid = 0;
#ifdef _OPENMP
    omp_set_num_threads(PREFILL_THREADS);
        const int ompThreads = omp_get_max_threads();
#else
    const int ompThreads = 1;
#endif

    TIMING_START("inserting "<<expectedSize<<" keys with "<<ompThreads<<" threads");
#pragma omp parallel
    {
#ifdef _OPENMP
        const int tid = omp_get_thread_num();
            g->dsAdapter->initThread(tid);
            binding_bindThread(tid);
#else
        const int tid = 0;
        g->dsAdapter->initThread(tid);
#endif

#pragma omp for schedule(dynamic, 100000)
        for (size_t i=0;i<expectedSize;++i) {
            test_type key = g->keygens[tid]->next();
            //test_type key = g->rngs[tid].next(MAXKEY) + 1;
            GSTATS_ADD(tid, num_inserts, 1);
            if (g->dsAdapter->INSERT_FUNC(tid, key, KEY_TO_VALUE(key)) == g->dsAdapter->getNoValue()) {
                GSTATS_ADD(tid, key_checksum, key);
                GSTATS_ADD(tid, prefill_size, 1);

                // monitor prefilling progress (completely optional!!)
                if ((tid == 0) && (GSTATS_GET(tid, prefill_size) % 1000000) == 0) {
                    double elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - g->prefillStartTime).count();
                    //double percent_done = GSTATS_GET_STAT_METRICS(prefill_size, TOTAL)[0].sum / (double) expectedSize;
                    double percent_done = GSTATS_GET(tid, prefill_size) * ompThreads / (double) expectedSize;
                    double magic_error_multiplier = (1+(1-percent_done)*1.25); // derived experimentally using huge trees. super rough and silly linear estimator for what is clearly a curve...
                    double total_estimate_ms = magic_error_multiplier * elapsed_ms / percent_done;
                    double remaining_ms = total_estimate_ms - elapsed_ms;
                    printf("tid=%d thread_prefill_amount=%lld percent_done_estimate=%.1f elapsed_s=%.0f est_remaining_s=%.0f / %0.f\n", tid, GSTATS_GET(tid, prefill_size), (100*percent_done), (elapsed_ms / 1000), (remaining_ms / 1000), (total_estimate_ms / 1000));
                    fflush(stdout); // for some reason the above is stubborn and doesn't print until too late (to watch progress) if i don't flush explicitly.
                }
            } else {
                --i;
                continue; // retry
            }
        }
    }
    TIMING_STOP;

    auto prefillEndTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(prefillEndTime-prefillStartTime).count();
    GSTATS_PRINT;
    const long totalSuccUpdates = GSTATS_GET_STAT_METRICS(num_inserts, TOTAL)[0].sum + GSTATS_GET_STAT_METRICS(num_deletes, TOTAL)[0].sum;
    g->prefillKeySum = GSTATS_GET_STAT_METRICS(key_checksum, TOTAL)[0].sum;
    g->prefillSize = GSTATS_GET_STAT_METRICS(prefill_size, TOTAL)[0].sum;
    COUTATOMIC("finished prefilling to size "<<expectedSize<<" keysum="<<g->prefillKeySum<<", performing "<<totalSuccUpdates<<" successful updates in "<<(elapsed/1000.)<<"s"<<std::endl);
    std::cout<<"pref_size="<<expectedSize<<std::endl;
    //std::cout<<"pref_millis="<<elapsed<<std::endl;
    GSTATS_CLEAR_ALL;
}

template<class GlobalsT>
void prefillWithUpdates(GlobalsT * g, int64_t expectedSize) {
    std::cout<<"Info: prefilling using UPDATES (ins & del)."<<std::endl;
    std::chrono::time_point<std::chrono::high_resolution_clock> prefillStartTime = std::chrono::high_resolution_clock::now();

    const double PREFILL_THRESHOLD = 0.02;
    const int MAX_ATTEMPTS = 10000;

    long long totalThreadsPrefillElapsedMillis = 0;

    std::thread * threads[MAX_THREADS_POW2];

    int sz = 0;
    int attempts;
    for (attempts=0;attempts<MAX_ATTEMPTS;++attempts) {
        INIT_ALL;

        // start all threads
        for (int i=0;i<PREFILL_THREADS;++i) {
            threads[i] = new std::thread(thread_prefill_with_updates<GlobalsT>, g, i);
        }

        TRACE COUTATOMIC("main thread: waiting for threads to START prefilling running="<<g->running<<std::endl);
        while (g->running < PREFILL_THREADS) {}
        TRACE COUTATOMIC("main thread: starting prefilling timer..."<<std::endl);
        g->startTime = std::chrono::high_resolution_clock::now();

        g->prefillIntervalElapsedMillis = 0;
        __sync_synchronize();
        g->start = true;

        /**
         * START INFINITE LOOP DETECTION CODE
         */
        // amount of time for main thread to wait for children threads
        timespec tsExpected;
        tsExpected.tv_sec = 0;
        tsExpected.tv_nsec = g->PREFILL_INTERVAL_MILLIS * ((__syscall_slong_t) 1000000);
        // short nap
        timespec tsNap;
        tsNap.tv_sec = 0;
        tsNap.tv_nsec = 10000000; // 10ms

        nanosleep(&tsExpected, NULL);
        g->done = true;
        __sync_synchronize();

        const long MAX_NAPPING_MILLIS = 5000;
        g->elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - g->startTime).count();
        g->elapsedMillisNapping = 0;
        while (g->running > 0 && g->elapsedMillisNapping < MAX_NAPPING_MILLIS) {
            nanosleep(&tsNap, NULL);
            g->elapsedMillisNapping = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - g->startTime).count() - g->elapsedMillis;
        }
        if (g->running > 0) {
            COUTATOMIC(std::endl);
            COUTATOMIC("Validation FAILURE: "<<g->running<<" non-responsive thread(s) [during prefill]"<<std::endl);
            COUTATOMIC(std::endl);
            exit(-1);
        }
        /**
         * END INFINITE LOOP DETECTION CODE
         */

        TRACE COUTATOMIC("main thread: waiting for threads to STOP prefilling running="<<g->running<<std::endl);
        while (g->running > 0) {}

        for (int i=0;i<PREFILL_THREADS;++i) {
            threads[i]->join();
            delete threads[i];
        }

        g->start = false;
        g->done = false;

        sz = GSTATS_OBJECT_NAME.get_sum<long long>(prefill_size);
        totalThreadsPrefillElapsedMillis += g->prefillIntervalElapsedMillis;
        if (sz >= (size_t) expectedSize*(1-PREFILL_THRESHOLD)) {
            break;
        } else {
            auto currTime = std::chrono::high_resolution_clock::now();
            auto totalElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currTime-prefillStartTime).count();
            std::cout << " finished prefilling round "<<attempts<<" with ds size: " << sz << " total elapsed time "<<(totalElapsed/1000.)<<"s"<<std::endl;
            std::cout<<"pref_round_size="<<sz<<std::endl;
        }

        DEINIT_ALL;
    }

    if (attempts >= MAX_ATTEMPTS) {
        std::cerr<<"ERROR: could not prefill to expected size "<<expectedSize<<". reached size "<<sz<<" after "<<attempts<<" attempts"<<std::endl;
        exit(-1);
    }

    auto prefillEndTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(prefillEndTime-prefillStartTime).count();

    GSTATS_PRINT;
    const long totalSuccUpdates = GSTATS_GET_STAT_METRICS(num_inserts, TOTAL)[0].sum + GSTATS_GET_STAT_METRICS(num_deletes, TOTAL)[0].sum;
    g->prefillKeySum = GSTATS_GET_STAT_METRICS(key_checksum, TOTAL)[0].sum;
    g->prefillSize = GSTATS_GET_STAT_METRICS(prefill_size, TOTAL)[0].sum;
    COUTATOMIC("finished prefilling to size "<<sz<<" for expected size "<<expectedSize<<" keysum="<< g->prefillKeySum /*<<" dskeysum="<<g->ds->getKeyChecksum()<<" dssize="<<g->ds->getSize()*/<<", performing "<<totalSuccUpdates<<" successful updates in "<<(totalThreadsPrefillElapsedMillis/1000.) /*(elapsed/1000.)*/<<" seconds (total time "<<(elapsed/1000.)<<"s)"<<std::endl);
    std::cout<<"pref_size="<<sz<<std::endl;
//    std::cout<<"pref_millis="<<elapsed<<std::endl;
    GSTATS_CLEAR_ALL;
    GSTATS_CLEAR_VAL(timersplit_epoch, get_server_clock());
    GSTATS_CLEAR_VAL(timersplit_token_received, get_server_clock());
    GSTATS_CLEAR_VAL(timer_bag_rotation_start, get_server_clock());
}

size_t * prefillWithArrayConstruction(auto g, int64_t expectedSize) {
    std::cout<<"Info: prefilling using ARRAY CONSTRUCTION to expectedSize="<<expectedSize<<" w/MAXKEY="<<MAXKEY<<"."<<std::endl;
    if (MAXKEY < expectedSize) setbench_error("specified key range must be large enough to accommodate the specified prefill size");

    TIMING_START("creating key array");
    size_t sz = MAXKEY+2;
    const size_t DOES_NOT_EXIST = std::numeric_limits<size_t>::max();
    size_t * present = new size_t[sz];
#ifdef _OPENMP
    omp_set_num_threads(PREFILL_THREADS);
#endif
#pragma omp parallel for schedule(dynamic, 100000)
    for (size_t i=0;i<sz;++i) present[i]=DOES_NOT_EXIST;
    TIMING_STOP;

    TIMING_START("choosing random keys with present array");
#pragma omp parallel for schedule(dynamic, 100000)
    for (size_t i=0;i<expectedSize;++i) {
        retry:
#ifdef _OPENMP
        const int tid = omp_get_thread_num();
#else
        const int tid = 0;
#endif
        test_type key = g->keygens[tid]->next();
        //auto key = g->rngs[tid].next(MAXKEY) + 1;
        if (__sync_bool_compare_and_swap(&present[key], DOES_NOT_EXIST, key)) {
            GSTATS_ADD(tid, key_checksum, key);
            GSTATS_ADD(tid, size_checksum, 1);
        } else {
            goto retry;
        }
    }
    TIMING_STOP;

    TIMING_START("parallel sort to obtain keys to insert");
#ifdef _OPENMP
    __gnu_parallel::sort(present, present+sz);
#else
    std::sort(present, present+sz);
#endif
    TIMING_STOP;

    return present;
}

void createAndPrefillDataStructure(auto g, int64_t expectedSize) {
    if (PREFILL_THREADS == 0) {
        g->dsAdapter = new DS_ADAPTER_T(std::max(PREFILL_THREADS, TOTAL_THREADS), g->KEY_MIN, g->KEY_MAX, g->NO_VALUE, g->rngs);
        return;
    }

    if (expectedSize == -1) {
        const double expectedFullness = (INS+DEL ? INS / (double)(INS+DEL) : 0.5); // percent full in expectation
        expectedSize = (int64_t) (MAXKEY * expectedFullness);
    }

    // prefill data structure to mimic its structure in the steady state
    g->prefillStartTime = std::chrono::high_resolution_clock::now();

    // PREBUILD VIA PARALLEL ARRAY CONSTRUCTION
#ifdef PREFILL_BUILD_FROM_ARRAY
    auto present = prefillWithArrayConstruction(g, expectedSize);
        TIMING_START("constructing data structure");
        g->dsAdapter = new DS_ADAPTER_T(
                std::max(PREFILL_THREADS, TOTAL_THREADS), g->KEY_MIN, g->KEY_MAX, g->NO_VALUE, g->rngs,
                (test_type const *) present, (VALUE_TYPE const *) present, expectedSize, rand());
        TIMING_STOP;
        delete[] present;

    // PREBUILD VIA REPEATED CONCURRENT INSERT-ONLY TRIALS
#elif defined PREFILL_INSERTION_ONLY
    g->dsAdapter = new DS_ADAPTER_T(std::max(PREFILL_THREADS, TOTAL_THREADS), g->KEY_MIN, g->KEY_MAX, g->NO_VALUE, g->rngs);
        prefillWithInserts(g, expectedSize);

    // PREBUILD VIA REPEATED CONCURRENT INSERT-AND-DELETE TRIALS
#else
    g->dsAdapter = new DS_ADAPTER_T(std::max(PREFILL_THREADS, TOTAL_THREADS), g->KEY_MIN, g->KEY_MAX, g->NO_VALUE, g->rngs);
    prefillWithUpdates(g, expectedSize);
#endif

    // print total prefilling time
    std::cout<<"prefill_elapsed_ms="<<std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - g->prefillStartTime).count()<<std::endl;
}

template <class GlobalsT>
void thread_timed(GlobalsT * g, int __tid) {
    tid = __tid;

    binding_bindThread(tid);
    test_type garbage = 0;

    test_type * rqResultKeys = new test_type[RQSIZE+MAX_KEYS_PER_NODE];
    VALUE_TYPE * rqResultValues = new VALUE_TYPE[RQSIZE+MAX_KEYS_PER_NODE];

    //std::cout<<"tid="<<tid<<std::endl;
    INIT_THREAD(tid);
    papi_create_eventset(tid);
    __sync_fetch_and_add(&g->running, 1);
    __sync_synchronize();
    //std::cout<<"thread "<<__tid<<" wait for g->start running="<<g->running<<std::endl;
    while (!g->start) { sched_yield(); __sync_synchronize(); TRACE COUTATOMICTID("waiting to start"<<std::endl); } // wait to start
    GSTATS_SET(tid, time_thread_start, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - g->startTime).count());
    papi_start_counters(tid);
    int cnt = 0;
    int rq_cnt = 0;

    DURATION_START(tid);

    while (!g->done) {
        if (((++cnt) % OPS_BETWEEN_TIME_CHECKS) == 0 || (rq_cnt % RQS_BETWEEN_TIME_CHECKS) == 0) {
            auto __endTime = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(__endTime-g->startTime).count() >= std::abs(MILLIS_TO_RUN)) {
                __sync_synchronize();
                g->done = true;
                __sync_synchronize();
                break;
            }
        }

        VERBOSE if (cnt&&((cnt % 1000000) == 0)) COUTATOMICTID("op# "<<cnt<<std::endl);
        test_type key = g->keygens[tid]->next();
        //test_type key = g->rngs[tid].next(MAXKEY) + 1;
//        printf("    key=%d\n", key);
        double op = g->rngs[tid].next(100000000) / 1000000.;
        if (op < INS) {
            GSTATS_TIMER_RESET(tid, timer_latency);
            if (g->dsAdapter->INSERT_FUNC(tid, key, KEY_TO_VALUE(key)) == g->dsAdapter->getNoValue()) {
                GSTATS_ADD(tid, key_checksum, key);
                GSTATS_ADD(tid, size_checksum, 1);
            }
            GSTATS_TIMER_APPEND_ELAPSED(tid, timer_latency, latency_updates);
            GSTATS_ADD(tid, num_inserts, 1);
        } else if (op < INS+DEL) {
            GSTATS_TIMER_RESET(tid, timer_latency);
            if (g->dsAdapter->erase(tid, key) != g->dsAdapter->getNoValue()) {
                GSTATS_ADD(tid, key_checksum, -key);
                GSTATS_ADD(tid, size_checksum, -1);
            }
            GSTATS_TIMER_APPEND_ELAPSED(tid, timer_latency, latency_updates);
            GSTATS_ADD(tid, num_deletes, 1);
        } else if (op < INS+DEL+RQ) {
            // TODO: make this respect KeyGenerators for non-uniform distributions
            uint64_t _key = g->rngs[tid].next() % std::max(1, MAXKEY - RQSIZE) + 1;
            assert(_key >= 1);
            assert(_key <= MAXKEY);
            assert(_key <= std::max(1, MAXKEY - RQSIZE));
            assert(MAXKEY > RQSIZE || _key == 0);
            key = (test_type) _key;

            ++rq_cnt;
            size_t rqcnt;
            GSTATS_TIMER_RESET(tid, timer_latency);
            if (rqcnt = g->dsAdapter->rangeQuery(tid, key, key+RQSIZE-1, rqResultKeys, (VALUE_TYPE *) rqResultValues)) {
                garbage += rqResultKeys[0] + rqResultKeys[rqcnt-1]; // prevent rqResultValues and count from being optimized out
            }
            GSTATS_TIMER_APPEND_ELAPSED(tid, timer_latency, latency_rqs);
            GSTATS_ADD(tid, num_rq, 1);
        } else {
            //    GSTATS_TIMER_RESET(tid, timer_latency);
            if (g->dsAdapter->contains(tid, key)) {
            }
            //    GSTATS_TIMER_APPEND_ELAPSED(tid, timer_latency, latency_searches);
            GSTATS_ADD(tid, num_searches, 1);
        }
        GSTATS_ADD(tid, num_operations, 1);
    }
    __sync_fetch_and_add(&g->running, -1);
//    GSTATS_SET(tid, num_prop_thread_exit_time, get_server_clock() - g->startClockTicks);

    DURATION_END(tid, duration_all_ops);

    GSTATS_SET(tid, time_thread_terminate, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - g->startTime).count());

    SOFTWARE_BARRIER;
    papi_stop_counters(tid);
    SOFTWARE_BARRIER;

    while (g->running) { sched_yield(); __sync_synchronize(); }
    DEINIT_THREAD(tid);
    delete[] rqResultKeys;
    delete[] rqResultValues;
    g->garbage += garbage;
}

template <class GlobalsT>
void thread_rq(GlobalsT * g, int __tid) {
    tid = __tid;

    binding_bindThread(tid);
    test_type garbage = 0;

    test_type * rqResultKeys = new test_type[RQSIZE+MAX_KEYS_PER_NODE];
    VALUE_TYPE * rqResultValues = new VALUE_TYPE[RQSIZE+MAX_KEYS_PER_NODE];

    INIT_THREAD(tid);
    papi_create_eventset(tid);
    __sync_fetch_and_add(&g->running, 1);
    __sync_synchronize();
    while (!g->start) { sched_yield(); __sync_synchronize(); TRACE COUTATOMICTID("waiting to start"<<std::endl); } // wait to start
    GSTATS_SET(tid, time_thread_start, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - g->startTime).count());

    papi_start_counters(tid);
    int cnt = 0;
    while (!g->done) {
        if (((++cnt) % RQS_BETWEEN_TIME_CHECKS) == 0) {
            auto __endTime = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(__endTime-g->startTime).count() >= MILLIS_TO_RUN) {
                __sync_synchronize();
                g->done = true;
                __sync_synchronize();
                break;
            }
        }

        VERBOSE if (cnt&&((cnt % 1000000) == 0)) COUTATOMICTID("op# "<<cnt<<std::endl);
        // TODO: make this respect KeyGenerators for non-uniform distributions
        uint64_t _key = g->rngs[tid].next() % std::max(1, MAXKEY - RQSIZE) + 1;
        assert(_key >= 1);
        assert(_key <= MAXKEY);
        assert(_key <= std::max(1, MAXKEY - RQSIZE));
        assert(MAXKEY > RQSIZE || _key == 0);

        test_type key = (test_type) _key;
        size_t rqcnt;
        GSTATS_TIMER_RESET(tid, timer_latency);
        if (rqcnt = g->dsAdapter->rangeQuery(tid, key, key+RQSIZE-1, rqResultKeys, (VALUE_TYPE *) rqResultValues)) {
            garbage += rqResultKeys[0] + rqResultKeys[rqcnt-1]; // prevent rqResultValues and count from being optimized out
        }
        GSTATS_TIMER_APPEND_ELAPSED(tid, timer_latency, latency_rqs);
        GSTATS_ADD(tid, num_rq, 1);
        GSTATS_ADD(tid, num_operations, 1);
    }
    __sync_fetch_and_add(&g->running, -1);

    GSTATS_SET(tid, time_thread_terminate, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - g->startTime).count());

    SOFTWARE_BARRIER;
    papi_stop_counters(tid);
    SOFTWARE_BARRIER;

    //    GSTATS_SET(tid, num_prop_thread_exit_time, get_server_clock() - g->startClockTicks);
    while (g->running) {
        sched_yield(); // wait
    }

    DEINIT_THREAD(tid);
    delete[] rqResultKeys;
    delete[] rqResultValues;
    g->garbage += garbage;
}

template <class GlobalsT>
void trial(GlobalsT * g) {
    papi_init_program(TOTAL_THREADS);

    // create the actual data structure and prefill it to match the expected steady state
    createAndPrefillDataStructure(g, DESIRED_PREFILL_SIZE);

    // setup measured part of the experiment
    INIT_ALL;

    // TODO: reclaim all garbage floating in the record manager that was generated during prefilling, so it doesn't get freed at the start of the measured part of the execution? (maybe it's better not to do this, since it's realistic that there is some floating garbage in the steady state. that said, it's probably not realistic that it's all eligible for reclamation, first thing...)

    // precompute amount of time for main thread to wait for children threads
    timespec tsExpected;
    tsExpected.tv_sec = MILLIS_TO_RUN / 1000;
    tsExpected.tv_nsec = (MILLIS_TO_RUN % 1000) * ((__syscall_slong_t) 1000000);
    // precompute short nap time
    timespec tsNap;
    tsNap.tv_sec = 0;
    tsNap.tv_nsec = 10000000; // 10ms

    // start all threads
    std::thread * threads[MAX_THREADS_POW2];
    for (int i=0;i<TOTAL_THREADS;++i) {
        if (i < WORK_THREADS) {
            threads[i] = new std::thread(thread_timed<GlobalsT>, g, i);
        } else {
            threads[i] = new std::thread(thread_rq<GlobalsT>, g, i);
        }
    }

    while (g->running < TOTAL_THREADS) {
        TRACE COUTATOMIC("main thread: waiting for threads to START running="<<g->running<<std::endl);
    } // wait for all threads to be ready
    COUTATOMIC("main thread: starting timer..."<<std::endl);

    DEBUG_PRINT_ARENA_STATS;
    COUTATOMIC(std::endl);
    COUTATOMIC("###############################################################################"<<std::endl);
    COUTATOMIC("################################ BEGIN RUNNING ################################"<<std::endl);
    COUTATOMIC("###############################################################################"<<std::endl);
    COUTATOMIC(std::endl);

    SOFTWARE_BARRIER;
    g->startTime = std::chrono::high_resolution_clock::now();
    g->startClockTicks = get_server_clock();
    SOFTWARE_BARRIER;
    g->start = true;
    SOFTWARE_BARRIER;

    // join is replaced with sleeping, and kill threads if they run too long
    // method: sleep for the desired time + a small epsilon,
    //      then check "g->running" to see if we're done.
    //      if not, loop and sleep in small increments for up to 5s,
    //      and exit(-1) if running doesn't hit 0.

    if (MILLIS_TO_RUN > 0) {
        nanosleep(&tsExpected, NULL);
        SOFTWARE_BARRIER;
        g->done = true;
        __sync_synchronize();
    }

    DEBUG_PRINT_ARENA_STATS;
    COUTATOMIC(std::endl);
    COUTATOMIC("###############################################################################"<<std::endl);
    COUTATOMIC("################################## TIME IS UP #################################"<<std::endl);
    COUTATOMIC("###############################################################################"<<std::endl);
    COUTATOMIC(std::endl);

    const long MAX_NAPPING_MILLIS = (MAXKEY > 5e7 ? 120000 : 30000);
    g->elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - g->startTime).count();
    g->elapsedMillisNapping = 0;
    while (g->running > 0 && g->elapsedMillisNapping < MAX_NAPPING_MILLIS) {
        nanosleep(&tsNap, NULL);
        g->elapsedMillisNapping = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - g->startTime).count() - g->elapsedMillis;
    }

    if (g->running > 0) {
        COUTATOMIC(std::endl);
        COUTATOMIC("Validation FAILURE: "<<g->running<<" non-terminating thread(s) [did we exhaust physical memory and experience excessive slowdown due to swap mem?]"<<std::endl);
        COUTATOMIC(std::endl);
        COUTATOMIC("elapsedMillis="<<g->elapsedMillis<<" elapsedMillisNapping="<<g->elapsedMillisNapping<<std::endl);

        if (g->dsAdapter->validateStructure()) {
            std::cout<<"Structural validation OK"<<std::endl;
        } else {
            std::cout<<"Structural validation FAILURE."<<std::endl;
        }

#if defined USE_GSTATS && defined OVERRIDE_PRINT_STATS_ON_ERROR
        GSTATS_PRINT;
            std::cout<<std::endl;
#endif

        g->dsAdapter->printSummary();
#ifdef RQ_DEBUGGING_H
        DEBUG_VALIDATE_RQ(TOTAL_THREADS);
#endif
        exit(-1);
    }

    // join all threads
    COUTATOMIC("joining threads...");
    for (int i=0;i<TOTAL_THREADS;++i) {
        //COUTATOMIC("joining thread "<<i<<std::endl);
        threads[i]->join();
        delete threads[i];
    }

    COUTATOMIC(std::endl);
    COUTATOMIC("###############################################################################"<<std::endl);
    COUTATOMIC("################################# END RUNNING #################################"<<std::endl);
    COUTATOMIC("###############################################################################"<<std::endl);
    COUTATOMIC(std::endl);

    COUTATOMIC(((g->elapsedMillis+g->elapsedMillisNapping)/1000.)<<"s"<<std::endl);
    std::cout<<"gstats_timer_elapsed timer_bag_rotation_start="<<GSTATS_TIMER_ELAPSED(0, timer_bag_rotation_start)/1000000000.<<std::endl;

    papi_deinit_program();
    DEINIT_ALL;
}

void printExecutionTime(auto g) {
    auto programExecutionElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - g->programExecutionStartTime).count();
    std::cout<<"total_execution_walltime="<<(programExecutionElapsed/1000.)<<"s"<<std::endl;
}

void printOutput(auto g) {
    std::cout<<"PRODUCING OUTPUT"<<std::endl;
#ifdef USE_TREE_STATS
    auto timeBeforeTreeStats = std::chrono::high_resolution_clock::now();
    auto treeStats = g->dsAdapter->createTreeStats(g->KEY_MIN, g->KEY_MAX);
    auto timeAfterTreeStats = std::chrono::high_resolution_clock::now();
    auto elapsedTreeStats = std::chrono::duration_cast<std::chrono::milliseconds>(timeAfterTreeStats-timeBeforeTreeStats).count();
    std::cout<<std::endl;
    std::cout<<"tree_stats_computeWalltime="<<(elapsedTreeStats/1000.)<<"s"<<std::endl;
    std::cout<<std::endl;
    //std::cout<<"size_nodes="<<
    std::cout<<treeStats->toString()<<std::endl;
#endif
    g->dsAdapter->printSummary(); // can put this before GSTATS_PRINT to help some hacky debug code in reclaimer_ebr_token route some information to GSTATS_ to be printed. not a big deal, though.

#ifdef USE_GSTATS
    GSTATS_PRINT;
    std::cout<<std::endl;

//    for (int threadID = 0; threadID < TOTAL_THREADS; ++threadID) {
//        for (int index=0;index<10;++index) {
//            auto startTime = GSTATS_GET_IX(threadID, thread_reclamation_start, index);
//            auto endTime = GSTATS_GET_IX(threadID, thread_reclamation_end, index);
//            std::cout<<"tid="<<threadID<<" startTime="<<startTime<<" endTime="<<endTime<<std::endl;
//        }
//    }

    std::cout<<std::endl;
#endif

    long long threadsKeySum = 0;
    long long threadsSize = 0;

#ifdef USE_GSTATS
    {
        threadsKeySum = GSTATS_GET_STAT_METRICS(key_checksum, TOTAL)[0].sum + g->prefillKeySum;
        threadsSize = GSTATS_GET_STAT_METRICS(size_checksum, TOTAL)[0].sum + g->prefillSize;
#ifdef USE_TREE_STATS
        long long dsKeySum = treeStats->getSumOfKeys();
        long long dsSize = treeStats->getKeys();
#endif
        std::cout<<"threads_final_keysum="<<threadsKeySum<<std::endl;
        std::cout<<"threads_final_size="<<threadsSize<<std::endl;
#ifdef USE_TREE_STATS
        std::cout<<"final_keysum="<<dsKeySum<<std::endl;
        std::cout<<"final_size="<<dsSize<<std::endl;
        if (threadsKeySum == dsKeySum && threadsSize == dsSize) {
            std::cout<<"validate_result=success"<<std::endl;
            std::cout<<"Validation OK."<<std::endl;
        } else {
            std::cout<<"validate_result=fail"<<std::endl;
            std::cout<<"Validation FAILURE: threadsKeySum="<<threadsKeySum<<" dsKeySum="<<dsKeySum<<" threadsSize="<<threadsSize<<" dsSize="<<dsSize<<std::endl;
            std::cout<<"Validation comment: data structure is "<<(dsSize > threadsSize ? "LARGER" : "SMALLER")<<" than it should be according to the operation return values"<<std::endl;
            printExecutionTime(g);

            exit(-1);
        }
#endif
    }
#endif

    if (g->dsAdapter->validateStructure()) {
        std::cout<<"Structural validation OK."<<std::endl;
    } else {
        std::cout<<"Structural validation FAILURE."<<std::endl;
        printExecutionTime(g);
        exit(-1);
    }

    long long totalAll = 0;

#ifdef USE_GSTATS
    {
        const long long totalSearches = GSTATS_GET_STAT_METRICS(num_searches, TOTAL)[0].sum;
        const long long totalRQs = GSTATS_GET_STAT_METRICS(num_rq, TOTAL)[0].sum;
        const long long totalQueries = totalSearches + totalRQs;
        const long long totalInserts = GSTATS_GET_STAT_METRICS(num_inserts, TOTAL)[0].sum;
        const long long totalDeletes = GSTATS_GET_STAT_METRICS(num_deletes, TOTAL)[0].sum;
        const long long totalUpdates = totalInserts + totalDeletes;

        const double SECONDS_TO_RUN = (MILLIS_TO_RUN)/1000.;
        totalAll = totalUpdates + totalQueries;
        const long long throughputSearches = (long long) (totalSearches / SECONDS_TO_RUN);
        const long long throughputRQs = (long long) (totalRQs / SECONDS_TO_RUN);
        const long long throughputQueries = (long long) (totalQueries / SECONDS_TO_RUN);
        const long long throughputUpdates = (long long) (totalUpdates / SECONDS_TO_RUN);
        const long long throughputAll = (long long) (totalAll / SECONDS_TO_RUN);

        COUTATOMIC(std::endl);
        COUTATOMIC("total_find="<<totalSearches<<std::endl);
        COUTATOMIC("total_rq="<<totalRQs<<std::endl);
        COUTATOMIC("total_inserts="<<totalInserts<<std::endl);
        COUTATOMIC("total_deletes="<<totalDeletes<<std::endl);
        COUTATOMIC("total_updates="<<totalUpdates<<std::endl);
        COUTATOMIC("total_queries="<<totalQueries<<std::endl);
        COUTATOMIC("total_ops="<<totalAll<<std::endl);
        COUTATOMIC("find_throughput="<<throughputSearches<<std::endl);
        COUTATOMIC("rq_throughput="<<throughputRQs<<std::endl);
        COUTATOMIC("update_throughput="<<throughputUpdates<<std::endl);
        COUTATOMIC("query_throughput="<<throughputQueries<<std::endl);
        COUTATOMIC("total_throughput="<<throughputAll<<std::endl);
        COUTATOMIC(std::endl);

        COUTATOMIC(std::endl);
        COUTATOMIC("total find                    : "<<totalSearches<<std::endl);
        COUTATOMIC("total rq                      : "<<totalRQs<<std::endl);
        COUTATOMIC("total inserts                 : "<<totalInserts<<std::endl);
        COUTATOMIC("total deletes                 : "<<totalDeletes<<std::endl);
        COUTATOMIC("total updates                 : "<<totalUpdates<<std::endl);
        COUTATOMIC("total queries                 : "<<totalQueries<<std::endl);
        COUTATOMIC("total ops                     : "<<totalAll<<std::endl);
        COUTATOMIC("find throughput               : "<<throughputSearches<<std::endl);
        COUTATOMIC("rq throughput                 : "<<throughputRQs<<std::endl);
        COUTATOMIC("update throughput             : "<<throughputUpdates<<std::endl);
        COUTATOMIC("query throughput              : "<<throughputQueries<<std::endl);
        COUTATOMIC("total throughput              : "<<throughputAll<<std::endl);
        COUTATOMIC(std::endl);
    }
#endif

    COUTATOMIC("elapsed milliseconds          : "<<g->elapsedMillis<<std::endl);
    COUTATOMIC("napping milliseconds overtime : "<<g->elapsedMillisNapping<<std::endl);
    COUTATOMIC(std::endl);

//    g->dsAdapter->printSummary();

    // free ds
#if !defined NO_CLEANUP_AFTER_WORKLOAD
    std::cout<<"begin delete ds..."<<std::endl;
    if (MAXKEY > 10000000) {
        std::cout<<"    SKIPPING deletion of data structure to save time! (because key range is so large)"<<std::endl;
    } else {
        delete g->dsAdapter;
    }
    std::cout<<"end delete ds."<<std::endl;
#endif

    papi_print_counters(totalAll);
#ifdef USE_TREE_STATS
    delete treeStats;
#endif

#if !defined NDEBUG
    std::cout<<"WARNING: NDEBUG is not defined, so experiment results may be affected by assertions and debug code."<<std::endl;
#endif
#if defined MEASURE_REBUILDING_TIME || defined MEASURE_TIMELINE_STATS || defined RAPID_RECLAMATION
    std::cout<<"WARNING: one or more of MEASURE_REBUILDING_TIME | MEASURE_TIMELINE_STATS | RAPID_RECLAMATION are defined, which *may* affect experiments results."<<std::endl;
#endif
}

void main_continued_with_globals(auto g) {
    g->programExecutionStartTime = std::chrono::high_resolution_clock::now();

    // print object sizes, to help debugging/sanity checking memory layouts
    g->dsAdapter->printObjectSizes();

    // setup thread pinning/binding
    binding_configurePolicy(TOTAL_THREADS);

    // print actual thread pinning/binding layout
    std::cout<<"ACTUAL_THREAD_BINDINGS=";
    for (int i=0;i<TOTAL_THREADS;++i) {
        std::cout<<(i?",":"")<<binding_getActualBinding(i);
    }
    std::cout<<std::endl;
    if (!binding_isInjectiveMapping(TOTAL_THREADS)) {
        std::cout<<"ERROR: thread binding maps more than one thread to a single logical processor"<<std::endl;
        exit(-1);
    }

    // setup per-thread statistics
    std::cout<<std::endl;
    GSTATS_CREATE_ALL;
    std::cout<<std::endl;

    // initialize a few stat timers to the current time (since i split their values, and want a reasonably recent starting time for the first split)
    GSTATS_CLEAR_VAL(timersplit_epoch, get_server_clock());
    GSTATS_CLEAR_VAL(timersplit_token_received, get_server_clock());
    GSTATS_CLEAR_VAL(timer_bag_rotation_start, get_server_clock());

    trial(g);
    printOutput(g);

    binding_deinit();
    std::cout<<"garbage="<<g->garbage<<std::endl; // to prevent certain steps from being optimized out
    GSTATS_DESTROY;

    printExecutionTime(g);
    delete g;
}

int main(int argc, char** argv) {
    if (argc == 1) {
        std::cout<<std::endl;
        std::cout<<"Example usage:"<<std::endl;
        std::cout<<"LD_PRELOAD=/path/to/libjemalloc.so "<<argv[0]<<" -nwork 64 -nprefill 64 -i 5 -d 5 -rq 0 -rqsize 1 -k 2000000 -nrq 0 -t 3000 -pin 0-15,32-47,16-31,48-63"<<std::endl;
        std::cout<<std::endl;
        std::cout<<"This command will benchmark the data structure corresponding to this binary with 64 threads repeatedly performing 5% key-inserts and 5% key-deletes and 90% key-searches (and 0% range queries with range query size set to a dummy value of 1 key), on random keys from the key range [0, 2000000), for 3000 ms. The data structure is initially prefilled by 64 threads to contain half of the key range. The -pin argument causes threads to be pinned. The specified thread pinning order is for one particular 64 thread system. (Try running ``lscpu'' and looking at ``NUMA node[0-9]'' for a reasonable pinning order.)"<<std::endl;
        return 1;
    }

    std::cout<<"binary="<<argv[0]<<std::endl;

    // setup default args
    PREFILL_THREADS = 0;
    MILLIS_TO_RUN = 1000;
    RQ_THREADS = 0;
    WORK_THREADS = 4;
    RQSIZE = 0;
    RQ = 0;
    INS = 10;
    DEL = 10;
    MAXKEY = 100000;
    DESIRED_PREFILL_SIZE = -1;  // note: -1 means "use whatever would be expected in the steady state"
    // to get NO prefilling, set -nprefill 0
    KeyGeneratorDistribution distribution = KeyGeneratorDistribution::UNIFORM;

    // read command line args
    // example args: -i 25 -d 25 -k 10000 -rq 0 -rqsize 1000 -nprefill 8 -t 1000 -nrq 0 -nwork 8
    for (int i=1;i<argc;++i) {
        if (strcmp(argv[i], "-i") == 0) {
            INS = atof(argv[++i]);
        } else if (strcmp(argv[i], "-d") == 0) {
            DEL = atof(argv[++i]);
        } else if (strcmp(argv[i], "-rq") == 0) {
            RQ = atof(argv[++i]);
        } else if (strcmp(argv[i], "-rqsize") == 0) {
            RQSIZE = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-k") == 0) {
            MAXKEY = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-nrq") == 0) {
            RQ_THREADS = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-nwork") == 0) {
            WORK_THREADS = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-nprefill") == 0) { // num threads to prefill with
            PREFILL_THREADS = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-prefillsize") == 0) {
            DESIRED_PREFILL_SIZE = atol(argv[++i]);
        } else if (strcmp(argv[i], "-dist-zipf") == 0) {
            distribution = KeyGeneratorDistribution::ZIPF;
        } else if (strcmp(argv[i], "-dist-uniform") == 0) {
            distribution = KeyGeneratorDistribution::UNIFORM; // default behaviour
        } else if (strcmp(argv[i], "-t") == 0) {
            MILLIS_TO_RUN = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-pin") == 0) { // e.g., "-pin 1.2.3.8-11.4-7.0"
            binding_parseCustom(argv[++i]); // e.g., "1.2.3.8-11.4-7.0"
            std::cout<<"parsed custom binding: "<<argv[i]<<std::endl;
        } else {
            std::cout<<"bad argument "<<argv[i]<<std::endl;
            exit(1);
        }
    }

    TOTAL_THREADS = WORK_THREADS + RQ_THREADS;

    // print used args
    PRINTS(DS_TYPENAME);
    PRINTS(FIND_FUNC);
    PRINTS(INSERT_FUNC);
    PRINTS(ERASE_FUNC);
    PRINTS(RQ_FUNC);
    PRINTS(RECLAIM);
    PRINTS(ALLOC);
    PRINTS(POOL);
    PRINTS(MAX_THREADS_POW2);
    PRINTS(CPU_FREQ_GHZ);
    PRINTI(MILLIS_TO_RUN);
    PRINTI(INS);
    PRINTI(DEL);
    PRINTI(RQ);
    PRINTI(RQSIZE);
    PRINTI(MAXKEY);
    PRINTI(PREFILL_THREADS);
    PRINTI(DESIRED_PREFILL_SIZE);
    PRINTI(TOTAL_THREADS);
    PRINTI(WORK_THREADS);
    PRINTI(RQ_THREADS);
    PRINTI(distribution);

    switch (distribution) {
        case UNIFORM: {
            main_continued_with_globals(new globals_t<KeyGeneratorUniform<test_type>>(MAXKEY, distribution));
        } break;
        case ZIPF: {
            main_continued_with_globals(new globals_t<KeyGeneratorZipf<test_type>>(MAXKEY, distribution));
        } break;
        default: {
            setbench_error("invalid case");
        } break;
    }
    return 0;
}
