/******************************************************************************
 * Copyright (c) 2014, Pedro Ramalhete, Andreia Correia
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Concurrency Freaks nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************
 */
#ifndef _CLH_RWLOCK_H_
#define _CLH_RWLOCK_H_

#include <atomic>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>

typedef struct clh_rwlock_node_ clh_rwlock_node_t;
using namespace std;
struct clh_rwlock_node_
{
    std::atomic<char> succ_must_wait;
};

typedef struct
{
    clh_rwlock_node_t * mynode;
    char padding1[64];  // To avoid false sharing with the tail
    std::atomic<clh_rwlock_node_t *> tail;
    char padding2[64];  // No point in having false-sharing with the tail
    std::atomic<long long> readers_counter;
} clh_rwlock_t;


void clh_rwlock_init(clh_rwlock_t * self);
void clh_rwlock_destroy(clh_rwlock_t * self);
void clh_rwlock_readlock(clh_rwlock_t * self);
void clh_rwlock_readunlock(clh_rwlock_t * self);
void clh_rwlock_writelock(clh_rwlock_t * self);
void clh_rwlock_writeunlock(clh_rwlock_t * self);

static clh_rwlock_node_t * clh_rwlock_create_node(char succ_must_wait)
{
    clh_rwlock_node_t * new_node = (clh_rwlock_node_t *)malloc(sizeof(clh_rwlock_node_t));
    std::atomic_store_explicit(&new_node->succ_must_wait, succ_must_wait, memory_order_relaxed);
    return new_node;
}


/*
 * Initializes the rwlock, creating a sentinel node.
 *
 * Progress Condition: Wait-Free Population Oblivious
 */
void clh_rwlock_init(clh_rwlock_t * self)
{
    // We create the first sentinel node unlocked, with succ_must_wait=0
    clh_rwlock_node_t * node = clh_rwlock_create_node(0);
    self->mynode = node;
    std::atomic_store(&self->readers_counter, 0LL);
    std::atomic_store(&self->tail, node);
}


/*
 * Destroy the rwlock, clearing all memory.
 * You must be sure that there are no other threads currently holding
 * the lock or attempting to.
 *
 * Progress Condition: Wait-Free Population Oblivious
 */
void clh_rwlock_destroy(clh_rwlock_t * self)
{
    std::atomic_store(&self->readers_counter, 0LL);
    free(std::atomic_load(&self->tail));
}


/*
 * Locks the rwlock for the current thread. Will wait for other threads
 * that did the std::atomic_exchange() before this one.
 *
 * Progress Condition: Blocking
 */
void clh_rwlock_readlock(clh_rwlock_t * self)
{
    // Create the new node locked by default, setting succ_must_wait=1
    clh_rwlock_node_t *mynode = clh_rwlock_create_node(1);
    clh_rwlock_node_t *prev = std::atomic_exchange(&self->tail, mynode);

    // This thread's node is now in the queue, so wait until it is its turn
    char prev_islocked = std::atomic_load_explicit(&prev->succ_must_wait, memory_order_relaxed);
    if (prev_islocked) {
        while (prev_islocked) {
            sched_yield();  // Replace this with thrd_yield() if you use <threads.h>
            prev_islocked = std::atomic_load(&prev->succ_must_wait);
        }
    }

    // Incrementing the readers_counter will prevent a Writer from going in
    std::atomic_fetch_add(&self->readers_counter, 1LL);

    // This will allow the next thread to go in, but only if it is a Reader
    std::atomic_store(&mynode->succ_must_wait, '\0');

    // This thread has acquired the lock and it is now safe to
    // cleanup the memory of the previous node.
    free(prev);
}


/*
 *
 * Progress Condition: Lock-Free, or Wait-Free Population Oblivious (on x86)
 */
void clh_rwlock_readunlock(clh_rwlock_t * self)
{
    long long prev_counter = std::atomic_fetch_add(&self->readers_counter, -1LL);
    if (prev_counter == 0) {
        // ERROR: This will occur if unlock() is called without a lock()
        return;
    }
}



/*
 * Locks the rwlock for the current thread. Will wait for other threads
 * that did the std::atomic_exchange() before this one.
 *
 * Progress Condition: Blocking
 */
void clh_rwlock_writelock(clh_rwlock_t * self)
{
    // Create the new node locked by default, setting succ_must_wait=1
    clh_rwlock_node_t *mynode = clh_rwlock_create_node(1);
    clh_rwlock_node_t *prev = std::atomic_exchange(&self->tail, mynode);

    // This thread's node is now in the queue, so wait until it is its turn
    char prev_islocked = std::atomic_load_explicit(&prev->succ_must_wait, memory_order_relaxed);
    if (prev_islocked) {
        while (prev_islocked) {
            sched_yield();  // Replace this with thrd_yield() if you use <threads.h>
            prev_islocked = std::atomic_load(&prev->succ_must_wait);
        }
    }

    // Even though succ_must_wait is 0, there may be unfinished Readers, so spin/wait
    // until they're over.
    long long readers_counter = std::atomic_load_explicit(&self->readers_counter, memory_order_relaxed);
    if (readers_counter != 0) {
        while (readers_counter != 0) {
            sched_yield();
            readers_counter = std::atomic_load(&self->readers_counter);
        }
    }
    // This thread has acquired the lock

    self->mynode = mynode;
    free(prev);
}


/*
 *
 * Progress Condition: Wait-Free Population Oblivious
 */
void clh_rwlock_writeunlock(clh_rwlock_t * self)
{
    std::atomic_store(&self->mynode->succ_must_wait, '\0');
}

#endif /* _CLH_RWLOCK_H_ */