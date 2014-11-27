#include <limits.h>
#include "processor.h"

namespace favor {
    namespace processor {
        namespace {

            enum ResultType {AVG_CHARS, AVG_RESPONSE, TOTAL_CHARS, TOTAL_RESPONSE, TOTAL_MESSAGES};

            size_t resultKey(ResultType t,  long contactId, long fromDate, long untilDate){
                //Shift bits here so identical values don't cancel each other out
                //Contact IDs and resultType are small so get shifted left
                return (std::hash<long>()(fromDate) ^
                        (std::hash<long>()(contactId) << 1) ^
                        ((std::hash<long>()(untilDate) << 1) >> 1) ^
                        (std::hash<long>()(t) << 2));
            }

            class Result {
            private:
                const ResultType type;
                long fromDate;
                long untilDate;
                long contactId;
                const shared_ptr<void> data; //http://stackoverflow.com/questions/5913396/why-do-stdshared-ptrvoid-work - wizardy.
                // TODO: verification of wizardy, specifically just write a tiny test with similar assignment and make sure destructors are firing properly
                Result(ResultType t, long contact, long fromD, long untilD) : type(t), fromDate(fromD), untilDate(untilD), contactId(contact) {}

                template <typename T>
                void setValue(T input){
                    data = std::make_shared<T>(input);
                }

            public:
                template <typename T>
                friend Result makeResult(ResultType t, long contact, long fromD, long untilD, T input);

                template<typename T>
                T getData(){
                    T* ptr = data.get();
                    return *ptr;
                }

                size_t key(){
                    return resultKey(type, fromDate, untilDate, contactId);
                }
            };

            template<typename T>
            Result makeResult(ResultType t, long contact, long fromD, long untilD, T input){
                Result ret(t, contact, fromD, untilD);
                ret.setValue<T>(input);
                return ret;
            }


            std::unordered_map<size_t, Result> cache;
            std::unordered_map<size_t, std::mutex> cacheLocks;


            template<typename T>
            void cacheResult(ResultType t, long contactId, long fromDate, long untilDate, T& input){
                Result res = makeResult(t, contactId, fromDate, untilDate, input);
                cacheLocks[res.key()].lock();
                cache[res.key()] = res;
                cacheLocks[res.key()].unlock();
            }

            template<typename T>
            T getResult(ResultType t, long contactId, long fromDate, long untilDate){
                long key = resultKey(t, contactId, fromDate, untilDate);
                cacheLocks[key].lock();
                T data = cache.at(key).getData<T>();
                cacheLocks[key].unlock();
                return data;
            }

            bool countResult(ResultType t, long contactId, long fromDate, long untilDate){
                long key = resultKey(t, contactId, fromDate, untilDate);
                cacheLocks[key].lock();
                bool result = cache.count(resultKey(t, contactId, fromDate, untilDate));
                cacheLocks[key].unlock();
                return result;

            }
        }




    }
}