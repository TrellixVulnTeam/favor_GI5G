/*
Copyright (C) 2015  Joshua Tanner (mindful.jt@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef favor_datalock_include
#define favor_datalock_include

#include "favor.h"
#include "address.h"

namespace favor {
    template <typename T>
    class DataLock {
    protected:
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

        bool valid() const {
            return mutex != NULL && data != NULL;
        }

        const T* operator->() const {
            if (!valid()) throw threadingException("Cannot reference invalid data lock");
            return data;
        }

        const T& operator*() const {
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