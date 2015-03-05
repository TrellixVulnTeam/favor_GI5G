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

        //Be very wary of messing with response time helper methods


        /*
            Input must be sorted
         */
        long percentile(float percent, vector<long>& input){
            if (input.size() == 0) {
                logger::warning("Cannot get percentile of empty vector, returning 0");
                return 0;
            }
            else std::sort(input.begin(), input.end()); //These have to be sorted for percentile to work properly and db sort order has them coming in in reverse
            size_t index = (size_t) std::round(((input.size() - 1) * percent)); //If we don't use round we drop the decimal
            return input[index];
        }

        long standardDeviationFloor(int deviations, const std::vector<std::pair<long,long>>& input){
            long mean = 0;
            for (auto it = input.begin(); it != input.end(); ++it) mean += it->second;
            mean /= input.size();

            long stddev = 0;
            for (auto it = input.begin(); it != input.end(); ++it) stddev += (it->second - mean) * (it->second - mean);

            stddev = std::sqrt(stddev / input.size());

            return mean - (stddev * deviations);
        }

        /*
            First holds response times for us, second for the other party.
         */
        shared_ptr<SentRec<vector<time_t>>> strippedDates(shared_ptr<vector<Message>> messages){
            shared_ptr<SentRec<vector<time_t>>> result = std::make_shared<SentRec<vector<time_t>>>();
            auto back = messages->rbegin(); //Iteration goes backwards because messages are sorted descending
            //Order is very important here, because we get meaningless results if we're not moving the same direction time is
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
                    if (it->sent) result->sent.push_back(it->date - back->date);
                    else result->received.push_back(it->date - back->date);
                    back = it;
                }
            }
            return result;
        }

        /*
            Tnput must be sorted. It was that or copying it though
            Compared to making a lap through the entire input vector for every element, this actually has a
            slightly worse worst case because we include a sort. That said, the average case is dramatically
            better.
         */
        vector<long> denseTimes(const vector<time_t>& input){

            //First is the value itself, second is its density
            std::vector<std::pair<long,long>> densities;
            size_t back = 0;

            for (auto it = input.begin(); it != input.end(); ++it) densities.push_back(std::pair<long, long>(*it, 0));

            for (size_t i = back; i != input.size(); ++i){
                while (input[i] - input[back] > DENSITY_DISTANCE) ++back; //Move the back index up to exclude any irrelevant points
                for (size_t j = back; j != i; ++j){
                    densities[i].second += 1;
                    densities[j].second +=1;
                }
            }

//            for (auto it = densities.begin(); it != densities.end(); ++it){
//                logger::info(as_string(it->first/60 - 5)+":"+as_string(it->second));
//            }

            long minDensity = standardDeviationFloor(1, densities);
            //logger::info("min:"+ as_string(minDensity));

            vector<long> out;
            for (auto it = densities.begin(); it != densities.end(); ++it){
                if (it->second >= minDensity) out.push_back(it->first); //>= instead of > is important for small cases with uniform density
            }

            return out;
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


        //For response times we compute both sent and rec, cache them separately, return the one requested

        SentRec<double> conversationalResponseTimeCompute(shared_ptr<vector<Message>> query){
            auto sentRecDates = strippedDates(query);

            double averageReceived = 0;
            double averageSent = 0;

            if (sentRecDates->received.size() > 0){
                vector<long> recResponseTimes = denseTimes(sentRecDates->received);
                if (recResponseTimes.size() > 0){
                    for (auto it = recResponseTimes.begin(); it != recResponseTimes.end(); ++it) averageReceived += *it;
                    averageReceived /= (double)recResponseTimes.size();
                }
            } else logger::warning("Received dates empty, returning 0");

            if (sentRecDates->sent.size() > 0){
                vector<long> sentResponseTimes = denseTimes(sentRecDates->sent);
                if (sentResponseTimes.size() > 0){
                    for (auto it = sentResponseTimes.begin(); it != sentResponseTimes.end(); ++it) averageSent += *it;
                    averageSent /= (double)sentResponseTimes.size();
                }
            } else logger::warning("Sent dates empty, returning 0");

            return SentRec<double>(averageSent, averageReceived);
        }

        double conversationalResponsetime(AccountManager *account, const Contact *c, time_t fromDate, time_t untilDate, bool sent){
            if (c == NULL) throw queryException("Cannot run response time queries with a null contact");
            if (countResult(AVG_CONV_RESPONSE, account, c, fromDate, untilDate, sent)){
                return getResult<double>(AVG_CONV_RESPONSE, account, c, fromDate, untilDate, sent);
            } else {
                auto query = reader::queryConversation(account, *c, KEY_DATE, fromDate, untilDate);
                SentRec<double> sentRecAvgs = conversationalResponseTimeCompute(query);

                cacheResult<double>(AVG_CONV_RESPONSE, account, c, fromDate, untilDate, true, sentRecAvgs.sent);
                cacheResult<double>(AVG_CONV_RESPONSE, account, c, fromDate, untilDate, false, sentRecAvgs.received);

                return sent ? sentRecAvgs.sent : sentRecAvgs.received;
            }
        }

        SentRec<long> responseTimeNintiethCompute(shared_ptr<vector<Message>> query){
            auto oursTheirs = strippedDates(query);
            long receivedNintieth = percentile(0.90, oursTheirs->received);
            long sentNintieth = percentile(0.90, oursTheirs->sent);

            return SentRec<long>(sentNintieth, receivedNintieth);
        }

        long responseTimeNintiethPercentile(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent){
            if (c == NULL) throw queryException("Cannot run response time queries with a null contact");
            if (countResult(RESPONSE_NINTIETH, account, c, fromDate, untilDate, sent)){
                return getResult<long>(RESPONSE_NINTIETH, account, c, fromDate, untilDate, sent);
            } else {
                auto query = reader::queryConversation(account, *c, KEY_DATE, fromDate, untilDate);
                SentRec<long> result = responseTimeNintiethCompute(query);

                cacheResult<long>(RESPONSE_NINTIETH, account, c, fromDate, untilDate, true, result.sent);
                cacheResult<long>(RESPONSE_NINTIETH, account, c, fromDate, untilDate, false, result.received);

                return sent ? result.sent : result.received;
            }
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

        long totalMessagecount(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent){
            if (countResult(TOTAL_MESSAGES, account, c, fromDate, untilDate, sent)) return getResult<long>(TOTAL_MESSAGES, account, c, fromDate, untilDate, sent);
            else {
                long value = c == NULL ? reader::countAll(account, fromDate, untilDate, sent) : reader::count(account, *c, fromDate, untilDate, sent);
                cacheResult<long>(TOTAL_MESSAGES, account, c, fromDate, untilDate, sent, value);
                return value;
            }

        }


    }
}