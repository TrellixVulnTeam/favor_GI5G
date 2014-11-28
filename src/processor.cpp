#include <limits.h>
#include "processor.h"

namespace favor {
    namespace processor {
        namespace {

            class Result;

            class ResultKey {
            private:
                const ResultType type;
                const long fromDate;
                const long untilDate;
                vector<long> contacts;
                vector<std::pair<string, MessageType>> accounts;

            public:
                friend class ResultKeyHasher;

                ResultKey(ResultType t, const vector<AccountManager*>& accs, const vector<Contact>& cons, long fromD, long untilD) : type(t), fromDate(fromD), untilDate(untilD) {
                    for (auto it = accs.begin(); it != accs.end(); ++it) accounts.push_back(std::pair<string, MessageType>((*it)->accountName, (*it)->type));
                    for (auto it = cons.begin(); it != cons.end(); ++it) contacts.push_back(it->id);
                }

                bool operator==(const ResultKey& other) const{
                    return (type == other.type &&
                            fromDate == other.fromDate &&
                            untilDate == other.untilDate &&
                            contacts == other.contacts &&
                            accounts == other.accounts);
                }

            };

            class Result {
            private:
                shared_ptr<void> data; //http://stackoverflow.com/questions/5913396/why-do-stdshared-ptrvoid-work - wizardy.
                // TODO: verification of wizardy, specifically just write a tiny test with similar assignment and make sure destructors are firing properly

                template <typename T>
                void setValue(T input){
                    data = std::make_shared<T>(input);
                }

            public:
                template<typename T>
                friend Result makeResult(T input);
                Result() {}

                template<typename T>
                T getData(){
                    T* ptr = (T*)data.get();
                    return *ptr;
                }

            };

            template<typename T>
            Result makeResult(T input){
                Result ret;
                ret.setValue<T>(input);
                return ret;
            }

            ResultKey makeResultKey(ResultType t, const vector<AccountManager*>* const accounts, const vector<Contact>* const contacts, long fromD, long untilD) {
                const vector<AccountManager*>& accountsInput = accounts != NULL ? *accounts : vector<AccountManager*>();
                const vector<Contact>& contactsInput = contacts != NULL ? *contacts : vector<Contact>();

                return ResultKey(t, accountsInput, contactsInput, fromD, untilD);
            }

            ResultKey makeResultKey(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD){
                vector<AccountManager*> accountsInput;
                vector<Contact> contactsInput;
                if (account != NULL) accountsInput.push_back(account);
                if (contact != NULL) contactsInput.push_back(*contact);

                return ResultKey(t, accountsInput, contactsInput, fromD, untilD);
            }

            struct ResultKeyHasher{
                std::size_t operator()(const ResultKey& k) const{
                    size_t ret = std::hash<long>()(k.fromDate) ^ std::hash<long>()(k.untilDate);
                    for (auto it = k.contacts.begin(); it != k.contacts.end(); ++it) ret ^ std::hash<long>()(*it);
                    for (auto it = k.accounts.begin(); it != k.accounts.end(); ++it) ret ^ std::hash<string>()(it->first);
                }
            };


            std::unordered_map<ResultKey, Result, ResultKeyHasher> cache;
            std::mutex cacheLock; //The possibility of rehashes means we have to lock around the whole datastructure

        }

        template<typename T>
        void cacheResult(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD, T input){
            ResultKey key = makeResultKey(t, account, contact, fromD, untilD);
            cacheLock.lock();
            cache[key] = makeResult<T>(input);
            cacheLock.unlock();
        }

        template<typename T>
        T getResult(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD){
            ResultKey key = makeResultKey(t, account, contact, fromD, untilD);
            cacheLock.lock();
            T ret = cache.at(key).getData<T>();
            cacheLock.unlock();
            return ret;
        }

        bool countResult(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD){
            ResultKey key = makeResultKey(t, account, contact, fromD, untilD);
            cacheLock.lock();
            bool count = cache.count(key);
            cacheLock.unlock();
            return count;
        }



        double averageCharcount(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent){
            if (countResult(AVG_CHARS, account, &c, fromDate, untilDate)) return getResult<double>(AVG_CHARS, account, &c, fromDate, untilDate);
            else {
                double value = account == NULL ? reader::averageAll(account, KEY_CHARCOUNT, fromDate, untilDate, sent) : reader::average(account, c, KEY_CHARCOUNT, fromDate, untilDate, sent);
                cacheResult<double>(AVG_CHARS, account, &c, fromDate, untilDate, value);
                return value;
            }
        }

        double averageResponsetime(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent){
            //TODO: dat response time computation tho...
        }

        long totalCharcount(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent){
            if (countResult(TOTAL_CHARS, account, &c, fromDate, untilDate)) return getResult<long>(TOTAL_CHARS, account, &c, fromDate, untilDate);
            else {
                long value = account == NULL ? reader::sumAll(account, KEY_CHARCOUNT, fromDate, untilDate, sent): reader::sum(account, c , KEY_CHARCOUNT, fromDate, untilDate, sent);
                cacheResult<long>(TOTAL_CHARS, account, &c, fromDate, untilDate, value);
                return value;
            }
        }


        long totalResponsetime(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent){

        }

        long totalMessagecount(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent){
//            if (countResult(TOTAL_CHARS, account, &c, fromDate, untilDate))
              //TODO: counting not implemented in reader yet
        }


    }
}