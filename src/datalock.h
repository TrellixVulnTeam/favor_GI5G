#ifndef favor_datalock_include
#define favor_datalock_include

#include "favor.h"
#include "address.h"

namespace favor {
    template <typename T>
    class DataLock {
    private:
        shared_ptr<std::lock_guard<std::mutex>> guard;
        T* data;
        bool* threadVar;

    public:

        void invalidate() {
            data = NULL;
            guard.reset();
            *threadVar = false;
        }

        bool valid() {
            return guard != NULL && data != NULL;
        }

        T* operator->(){
            if (!valid()) throw threadingException("Cannot reference invalid data lock");
            return data;
        }

        T operator*(){
            if (!valid()) throw threadingException("Cannot reference invalid data lock");
            return *data;
        }

        DataLock(std::mutex& mutex, bool* tvar, T* protected_data) {
            guard = std::make_shared<std::lock_guard<std::mutex>>(mutex);
            data = protected_data;
            threadVar = tvar;
        }

        DataLock(const favor::DataLock<T> &other) {
            guard = other.guard;
            data = other.data;
            threadVar = other.threadVar;
        }

        DataLock<T>& operator=(const favor::DataLock<T> &other) {
            guard = other.guard;
            data = other.data;
            threadVar = other.threadVar;
        }

        ~DataLock() {
            invalidate();
        }

        DataLock() = delete;

    };
}

#endif