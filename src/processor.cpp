#include "processor.h"

namespace favor {
    namespace processor {
        namespace {

            enum ResultType {AVG_CHARS, AVG_RESPONSE, TOTAL_CHARS, TOTAL_RESPONSE, TOTAL_MESSAGES};
            class Result {
            private:
                const ResultType type;
                long fromDate;
                long untilDate;
                long contactId;
                const shared_ptr<void> data; //TODO: if this doesn't work (or even if it does) we might be able to get away with a normal pointer here as long as we're careful
                //not to copy this object. remember we have noncopyable macros

            public:
                template<typename T>
                T& getData(){
                    T* ptr = data.get();
                    return *ptr;
                }

//                template<typename T>
//                void setData(T& input){
//                    data = std::make_shared<T>(input);
//                }

                //TODO: a hashcode method that gives us some sort of ID based on contactId, fromDate and untilDate. We can't produce something flawlessly unique without using
                //three times as many bits as fit in a long (becaues information loss) so this will just have to be our "best guess". hopefully we can find a data structuer that
                //handles collisions for us, but if not, on collision we look for the exact item by checking for perfect equality on these three attributes

                template<typename T>
                Result(ResultType t, long contact, long fromD, long untilD, T& input) :
                        fromDate(fromD), untilDate(untilD), contactId(contact), data(std::make_shared<T>(input)){}
            };

            template<typename T>
            void cacheResult(ResultType t, long contactId, long fromDate, long untilDate, T& input){
                //TODO: construct the result and drop it in the cache. we need to do some long and hard thinking about what data structure to use here because while we need to
                //be able to look for something by fromDate/untilDate/contactId, I'm not super interested in writing my own hash table just for this.
            }

            template<typename T>
            T getResult(ResultType t, long contactId, long fromDate, long untilDate){

            }

            bool countResult(ResultType t, long contactId, long fromDate, long untilDate){
                //TODO: look for the result in the cache, return true if it exists. Is there a more elegant way to put the check and get in the same method?
            }
        }
    }
}