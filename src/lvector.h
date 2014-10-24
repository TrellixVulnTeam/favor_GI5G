#ifndef favor_lvector_include
#define favor_lvector_include

#include "favor.h"

namespace favor {
    template<typename T>
    class LVector : public std::vector<T> {
    public:
        //TODO: we don't need our own lock classes, as I now realize halfway through that this will be far too ugly to roll from scratch. The thing to do is
        //to keep our claimWriteLock and claimReadLock methods but have them spit out boost shared_mutex classes that have already claimed their shared/unique
        //lock, and this claiming will serve as the blocking on the function anyway.
        class WriteLock : public std::lock_guard<std::mutex> {
            friend class LVector<T>;
            WriteLock() = delete;
            NONCOPYABLE(WriteLock)
            NONMOVEABLE(WriteLock)
            WriteLock(LVector<T>& p);
            //Don't need anything else here, this is a normal lock_guard around our writerLock

        };
        class ReadLock {
            //Note: Obviously if these ever outlive their parent, the behavior is undefined.
            friend class LVector<T>;
            LVector<T>& parent;
            ReadLock() = delete;
            NONCOPYABLE(ReadLock)
            NONMOVEABLE(ReadLock)
            ReadLock(LVector<T>& p);
            ~ReadLock();
        };

    private:
        std::mutex internalLock;
        std::mutex writerLock;
        int readers;
        LVector(const LVector<T>& l) = delete;

    public:
        shared_ptr<WriteLock> claimWriteLock();
        shared_ptr<ReadLock> claimReadLock();

    };
}
#endif