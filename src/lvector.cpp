#include "lvector.h"

namespace favor{
    template <typename T>
    LVector<T>::WriteLock::WriteLock(LVector<T>& p) : lock_guard(p.writerLock){}

    template <typename T>
    LVector<T>::ReadLock::ReadLock(LVector<T>& p) : parent(p){

    }

    template <typename T>
    LVector<T>::ReadLock::~ReadLock(){

    }

    template <typename T>
    shared_ptr<typename LVector<T>::WriteLock> LVector<T>::claimWriteLock() {
        return new WriteLock(*this);
    }

    template <typename T>
    shared_ptr<typename LVector<T>::ReadLock> LVector<T>::claimReadLock() {
        return new ReadLock(*this);
    }
}