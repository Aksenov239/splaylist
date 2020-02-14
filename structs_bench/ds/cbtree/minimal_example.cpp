/**
 * Author: Trevor Brown (me [at] tbrown [dot] pro).
 * Copyright 2018.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <iostream>
#include <limits>
#include <cassert>

#include "adapter.h"

int main(int argc, char** argv) {

    const int NUM_THREADS = 1;
    const int KEY_NEG_INFTY = std::numeric_limits<int>::min();
    const int KEY_POS_INFTY = std::numeric_limits<int>::max();
    void * const unused2 = NULL;
    RandomFNV1A * const unused3 = NULL;

    auto tree = new ds_adapter<int, void *>(NUM_THREADS, KEY_NEG_INFTY, KEY_POS_INFTY, unused2, unused3);

    const int threadID = 0;

    tree->initThread(threadID);

    void * oldVal = tree->insertIfAbsent(threadID, 7, (void *) 1020);
    assert(oldVal == tree->getNoValue());
    for (int i = 5; i < 9; i++)
        tree->insertIfAbsent(threadID, 2 * i, (void *) 1020);
        for (int i = 5; i < 9; i++)
            tree->insertIfAbsent(threadID, 2 * i + 1, (void *) 1020);

    bool result;
    tree->printSummary();
    for (int i = 0; i < 1000; i++)
    {
        result = tree->contains(threadID, 15);
        assert(result);
    }
    tree->printSummary();

    result = tree->contains(threadID, 8);
    assert(!result);

    void * val = tree->find(threadID, 7);
    assert(val == (void *) 1020);

    // val = tree->erase(threadID, 7);
    // assert(val == (void *) 1020);

    result = tree->contains(threadID, 7);
    assert(result);

    tree->deinitThread(threadID);

    delete tree;

    std::cout<<"Passed quick tests."<<std::endl;


    return 0;
}
