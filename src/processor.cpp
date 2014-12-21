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


            /*-------------
            Response time stuff
             ---------------*/
            const long DENSITY_DISTANCE = 15 * 60; //15 minutes in seconds
            const long STRIP_RESET_DISTANCE = DENSITY_DISTANCE; //For now these can be the same thing

        }

        //This is working correctly. Be wary of trying to mess with it
        /*
            First holds response times for us, second for the other party.
         */
        shared_ptr<std::pair<vector<time_t>,vector<time_t>>> strippedDates(shared_ptr<vector<Message>> messages){
            shared_ptr<std::pair<vector<time_t>,vector<time_t>>> result = std::make_shared<std::pair<vector<time_t>,vector<time_t>>>();
            auto back = messages->rbegin(); //Iteration goes backwards because messages are sorted descending
            for (auto it = back; it != messages->rend(); ++it){
                if (it == back) continue; //Important so we don't try and do anything with the end iterator on the first pass
                if (back->sent == it->sent){
                    //We know a strip needs to happen. The question is whether the new message is far enough ahead to strip the back one
                    if (it->date - back->date >= STRIP_RESET_DISTANCE){
                        //This is too far, drop the old messages
                        back = it;
                    } //Else we keep going and have simply ignored the message with the same date
                } else {
                    //Sent and received differ, so we add a response time and move the back pointer up
                    if (it->sent) result->first.push_back(it->date - back->date);
                    else result->second.push_back(it->date - back->date);
                    back = it;
                }
            }
            return result;
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



        double averageResponsetime(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent){
            //TODO: dat response time computation tho...
        }

        double averageCharcount(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent){
            if (countResult(AVG_CHARS, account, c, fromDate, untilDate, sent)){
                return getResult<double>(AVG_CHARS, account, c, fromDate, untilDate, sent);
            }
            else {
                double value = c == NULL ? reader::averageAll(account, KEY_CHARCOUNT, fromDate, untilDate, sent) : reader::average(account, *c, KEY_CHARCOUNT, fromDate, untilDate, sent);
                cacheResult<double>(AVG_CHARS, account, c, fromDate, untilDate, sent, value);
                return value;
            }
        }

        long totalCharcount(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent){
            if (countResult(TOTAL_CHARS, account, c, fromDate, untilDate, sent)) return getResult<long>(TOTAL_CHARS, account, c, fromDate, untilDate, sent);
            else {
                long value = c == NULL ? reader::sumAll(account, KEY_CHARCOUNT, fromDate, untilDate, sent): reader::sum(account, *c , KEY_CHARCOUNT, fromDate, untilDate, sent);
                cacheResult<long>(TOTAL_CHARS, account, c, fromDate, untilDate, sent, value);
                return value;
            }
        }


        long totalResponsetime(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent){

        }

        long totalMessagecount(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent){
            if (countResult(TOTAL_MESSAGES, account, c, fromDate, untilDate, sent)) return getResult<long>(TOTAL_MESSAGES, account, c, fromDate, untilDate, sent);
            else {
                long value = c == NULL ? reader::countAll(account, fromDate, untilDate, sent) : reader::count(account, *c, fromDate, untilDate, sent);
                cacheResult<long>(TOTAL_MESSAGES, account, c, fromDate, untilDate, sent, value);
            }

        }


    }
}