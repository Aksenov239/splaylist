#pragma once

#define CASWORD_BITS_TYPE casword_t

template <typename T>
struct casword {
private:
    CASWORD_BITS_TYPE volatile bits;
public:
    casword();

    T setInitVal(T other);

    operator T();

    T operator->();

    T getValue();

    void addToDescriptor(T oldVal, T newVal);
};

#include "kcas_reuse_htm_impl.h"

namespace kcas {
    KCASHTM<MAX_KCAS> instance;

    void writeInitPtr(casword_t volatile * addr, casword_t const newval) {
        return instance.writeInitPtr(addr, newval);
    }

    void writeInitVal(casword_t volatile * addr, casword_t const newval) {
        return instance.writeInitVal(addr, newval);
    }

    casword_t readPtr(casword_t volatile * addr) {
        return instance.readPtr(addr);
    }

    casword_t readVal(casword_t volatile * addr) {
        return instance.readVal(addr);
    }

    bool execute() {
        return instance.execute();
    }

    kcasptr_t getDescriptor() {
        return instance.getDescriptor();
    }

    void start() {
        return instance.start();
    }

    template<typename T>
    void add(casword<T> * caswordptr, T oldVal, T newVal) {
        return instance.add(caswordptr, oldVal, newVal);
    }

    template<typename T, typename... Args>
    void add(casword<T> * caswordptr, T oldVal, T newVal, Args... args) {
        instance.add(caswordptr, oldVal, newVal, args...);
    }

};

#include "casword.h"