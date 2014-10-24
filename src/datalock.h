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

    public:

        void invalidate() {
            data = NULL;
            guard.reset();
        }

        bool valid() {
            return guard != NULL && data != NULL;
        }

        T* operator->(){
            return data;
        }

        T operator*(){
            return *data;
        }

        DataLock(std::mutex& mutex, T* protected_data) {
            guard = std::make_shared<std::lock_guard<std::mutex>>(mutex);
            data = protected_data;
        }

        DataLock(const favor::DataLock<T> &other) {
            guard = other.guard;
            data = other.data;
        }

        DataLock<T>& operator=(const favor::DataLock<T> &other) {
            guard = other.guard;
            data = other.data;
        }

        ~DataLock() {
            invalidate();
        }

        DataLock() = delete;

    };
}

#endif