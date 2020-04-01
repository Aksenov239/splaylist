#ifndef __DCLC_RWLOCK_H__
#define __DCLC_RWLOCK_H__

#include <atomic>
#include <thread>

/*
 * TODO: Add the blabla here
 *
 * Disadvantages:
 * - Can't use this lock on code that can fork()
 * - Shouldn't use this on signal handler code (or any kind of lock for that matter)
 *
 *
 */

// Use 0 for writer's "unlocked" and 1 for "locked" state
#define DCLC_RWL_UNLOCKED    0
#define DCLC_RWL_LOCKED      1

// Cache line optimization constants
#define DCLC_CACHE_LINE          64               // Size in bytes of a cache line
#define DCLC_CACHE_PADD          (DCLC_CACHE_LINE-sizeof(std::atomic<int>))
#define DCLC_NUMBER_OF_CORES     32
#define DCLC_HASH_RATIO           3
#define DCLC_COUNTERS_RATIO      (DCLC_HASH_RATIO*DCLC_CACHE_LINE/sizeof(int))


/* This is not recursive/reentrant */
class DCLCRWLock {
public:
/**
 * Default constructor
 *
 */
DCLCRWLock ()
{
    int hw_cores = std::thread::hardware_concurrency();
    // Default number of cores assumes this machine has 8 cores
    if (hw_cores == 0) hw_cores = DCLC_NUMBER_OF_CORES;
    numCores = hw_cores;
    countersLength = numCores*DCLC_COUNTERS_RATIO;
    writersMutex.store(DCLC_RWL_UNLOCKED);
    readersCounters = new std::atomic<int>[countersLength];
    for (int idx = 0; idx < countersLength; idx += DCLC_COUNTERS_RATIO) {
        readersCounters[idx] = 0;
    }
}


/**
 * With this constructor the use can specify the number of cores on the system
 */
DCLCRWLock (int num_cores)
{
    this->numCores = num_cores;
    countersLength = num_cores*DCLC_COUNTERS_RATIO;
    writersMutex.store(DCLC_RWL_UNLOCKED);
    readersCounters = new std::atomic<int>[countersLength];
    for (int idx = 0; idx < countersLength; idx += DCLC_COUNTERS_RATIO) {
        readersCounters[idx] = 0;
    }
}


/**
 * Default destructor
 */
~DCLCRWLock ()
{
    delete[] readersCounters;
    writersMutex.store(DCLC_RWL_UNLOCKED);
}


/**
 * Hashes a number and returns the index in the array of Reader's counters
 *
 * Returns a random index to be used in readers_counters[]
 */
int thread2idx (void) {
    std::size_t tid = hashFunc(std::this_thread::get_id());
    return (int)((tid % numCores)*DCLC_COUNTERS_RATIO);
}


/**
 *
 */
void sharedLock (void)
{
    const int idx = thread2idx();
    while (true) {
        readersCounters[idx].fetch_add(1);
        if (writersMutex.load() == DCLC_RWL_UNLOCKED) {
            // Acquired lock in read-only mode
            return;
        } else {
            // A Writer has acquired the lock, must reset to 0 and wait
            readersCounters[idx].fetch_add(-1);
            while (writersMutex.load() == DCLC_RWL_LOCKED) {
                std::this_thread::yield();
            }
        }
    }
}


/**
 *
 *
 */
bool sharedUnlock (void)
{
    if (readersCounters[thread2idx()].fetch_add(-1) <= 0) {
        // ERROR: no matching lock() for this unlock()
        std::cout << "ERROR: no matching lock() for this unlock()\n";
        return false;
    } else {
        return true;
    }
}


/**
 *
 */
void exclusiveLock (void)
{
    int old = DCLC_RWL_UNLOCKED;
    // Try to acquire the write-lock
    while (!writersMutex.compare_exchange_strong(old, DCLC_RWL_LOCKED)) {
        std::this_thread::yield();
        old = DCLC_RWL_UNLOCKED;
    }
    // Write-lock was acquired, now wait for any running Readers to finish
    for (int idx = 0; idx < countersLength; idx += DCLC_COUNTERS_RATIO) {
        while (readersCounters[idx].load() > 0) {
            std::this_thread::yield();
        }
    }
}


/**
 *
 */
bool exclusiveUnlock (void)
{
	if (writersMutex.load(std::memory_order_relaxed) != DCLC_RWL_LOCKED) {
        // ERROR: Tried to unlock an non-locked write-lock
        std::cout << "********* ERROR: Tried to unlock an non-locked write-lock\n";
        return false;
    }
    writersMutex.store(DCLC_RWL_UNLOCKED);
    return true;
}


bool trySharedLock (void)
{
    const int tid = thread2idx();
    readersCounters[tid].fetch_add(1);
    if (writersMutex.load() == DCLC_RWL_UNLOCKED) {
        // Acquired lock in read-only mode
        return true;
    } else {
        // A Writer has acquired the lock, must reset to 0 and wait
        readersCounters[tid].fetch_add(-1);
        return false;
    }
}

/* TODO: implement the writer's trylock and the timeouts */

private:
    /* Hash Function for thread id to integer */
    std::hash<std::thread::id> hashFunc;
    /* Number of cores on the system */
    int          numCores;
    /* Length of readers_counters[] */
    int          countersLength;
    /* Distributed Counters for Readers */
    std::atomic<int>  *readersCounters;
    /* Padding */
    char               pad1[DCLC_CACHE_PADD];
    /* lock/unlocked in write-mode */
    std::atomic<int>   writersMutex;
};



#endif