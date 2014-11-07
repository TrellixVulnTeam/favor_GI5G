#ifndef favor_datalock_include
#define favor_datalock_include

#include "favor.h"
#include "address.h"

namespace favor {
    template <typename T>
    class DataLock {
    private:
        std::mutex* mutex;
        int* holderCount;
        T* data;

        void holderCheck(){
            if (valid()){
                if (*holderCount == 0) mutex->lock();
                *holderCount += 1;
            }
        }

    public:

        void invalidate() {
            //If there threadVar count is 0 after decrementing
            *holderCount -= 1;
            if (*holderCount == 0 ) mutex->unlock();
            mutex = NULL;
            data = NULL;
        }

        bool valid() {
            return mutex != NULL && data != NULL;
        }

        T* operator->(){
            if (!valid()) throw threadingException("Cannot reference invalid data lock");
            return data;
        }

        T operator*(){
            if (!valid()) throw threadingException("Cannot reference invalid data lock");
            return *data;
        }

        DataLock(std::mutex* mut, int* hcount, T* protected_data) {
            mutex = mut;
            data = protected_data;
            holderCount = hcount;
            holderCheck();
        }

        DataLock(const favor::DataLock<T> &other) {
            mutex = other.mutex;
            data = other.data;
            holderCount = other.holderCount;
            holderCheck();
        }

        DataLock<T>& operator=(const favor::DataLock<T> &other) {
            mutex = other.mutex;
            data = other.data;
            holderCount = other.holderCount;
            holderCheck();
        }

        ~DataLock() {
            invalidate();
        }

        DataLock() = delete;

    };
}

#endif