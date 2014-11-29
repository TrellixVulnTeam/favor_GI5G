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
                const bool sent;
                vector<long> contacts;
                vector<std::pair<string, MessageType>> accounts;

            public:
                friend class ResultKeyHasher;

                ResultKey(ResultType t, const vector<AccountManager*>& accs, const vector<Contact>& cons, long fromD, long untilD, bool s) :
                        type(t), fromDate(fromD), untilDate(untilD), sent(s) {
                    for (auto it = accs.begin(); it != accs.end(); ++it) accounts.push_back(std::pair<string, MessageType>((*it)->accountName, (*it)->type));
                    for (auto it = cons.begin(); it != cons.end(); ++it) contacts.push_back(it->id);
                }

                bool operator==(const ResultKey& other) const{
                    return (type == other.type &&
                            sent == other.sent &&
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

            ResultKey makeResultKey(ResultType t, const vector<AccountManager*>* const accounts, const vector<Contact>* const contacts, long fromD, long untilD, bool sent) {
                const vector<AccountManager*>& accountsInput = accounts != NULL ? *accounts : vector<AccountManager*>();
                const vector<Contact>& contactsInput = contacts != NULL ? *contacts : vector<Contact>();

                return ResultKey(t, accountsInput, contactsInput, fromD, untilD, sent);
            }

            ResultKey makeResultKey(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD, bool sent){
                vector<AccountManager*> accountsInput;
                vector<Contact> contactsInput;
                if (account != NULL) accountsInput.push_back(account);
                if (contact != NULL) contactsInput.push_back(*contact);

                return ResultKey(t, accountsInput, contactsInput, fromD, untilD, sent);
            }

            struct ResultKeyHasher{
                std::size_t operator()(const ResultKey& k) const{
                    //Sent determines the ordering here, but the operations on fromDate and untilDate are modeled after boost::hash_combine
                    //because this is very much not my field of expertise
                    size_t ret = k.sent ? std::hash<long>()(k.fromDate) : std::hash<long>()(k.untilDate);
                    if (k.sent) ret ^= std::hash<long>()(k.untilDate) + 0x9e3779b9 + (ret << 6) + (ret >> 2);
                    else ret ^= std::hash<long>()(k.fromDate) + 0x9e3779b9 + (ret << 6) + (ret >> 2);
                    for (auto it = k.contacts.begin(); it != k.contacts.end(); ++it) ret ^ std::hash<long>()(*it);
                    for (auto it = k.accounts.begin(); it != k.accounts.end(); ++it) ret ^ std::hash<string>()(it->first);
                }
            };


            std::unordered_map<ResultKey, Result, ResultKeyHasher> cache;
            std::mutex cacheLock; //The possibility of rehashes means we have to lock around the whole datastructure

        }

        template<typename T>
        void cacheResult(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD, bool sent, T input){
            ResultKey key = makeResultKey(t, account, contact, fromD, untilD, sent);
            cacheLock.lock();
            cache[key] = makeResult<T>(input);
            cacheLock.unlock();
        }

        template<typename T>
        T getResult(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD, bool sent){
            ResultKey key = makeResultKey(t, account, contact, fromD, untilD, sent);
            cacheLock.lock();
            T ret = cache.at(key).getData<T>();
            cacheLock.unlock();
            return ret;
        }

        bool countResult(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD, bool sent){
            ResultKey key = makeResultKey(t, account, contact, fromD, untilD, sent);
            cacheLock.lock();
            bool count = cache.count(key);
            cacheLock.unlock();
            return count;
        }



        double averageCharcount(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent){
            if (countResult(AVG_CHARS, account, &c, fromDate, untilDate, sent)){
                return getResult<double>(AVG_CHARS, account, &c, fromDate, untilDate, sent);
            }
            else {
                double value = account == NULL ? reader::averageAll(account, KEY_CHARCOUNT, fromDate, untilDate, sent) : reader::average(account, c, KEY_CHARCOUNT, fromDate, untilDate, sent);
                cacheResult<double>(AVG_CHARS, account, &c, fromDate, untilDate, value, sent);
                return value;
            }
        }

        double averageResponsetime(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent){
            //TODO: dat response time computation tho...
        }

        long totalCharcount(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent){
            if (countResult(TOTAL_CHARS, account, &c, fromDate, untilDate, sent)) return getResult<long>(TOTAL_CHARS, account, &c, fromDate, untilDate, sent);
            else {
                long value = account == NULL ? reader::sumAll(account, KEY_CHARCOUNT, fromDate, untilDate, sent): reader::sum(account, c , KEY_CHARCOUNT, fromDate, untilDate, sent);
                cacheResult<long>(TOTAL_CHARS, account, &c, fromDate, untilDate, value, sent);
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