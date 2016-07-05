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
#include "conversation_data.h"

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
            float index = (percent * input.size());
            if (floor(index) == index){
                return (input[((int)index)-1] + input[(int)index]) / 2;
            }
            else {
                return input[floor(index)];
            }
            DLOG("Index "+as_string((int)index));
            for (int i = 0; i < input.size(); ++i){
                DLOG("Num in Percentile Array: "+as_string(input[i]));
            }
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
        //TODO: see "Conversation Stripping" note, but we should implement an "optimistic" boolean to determine
        //whether we optimistically use the latest date to compute response time (as we do now), or pessimisticly
        //use the earliest one to compute it
        shared_ptr<SentRec<vector<time_t>>> strippedDates(const shared_ptr<const vector<Message>> messages){
            shared_ptr<SentRec<vector<time_t>>> result = std::make_shared<SentRec<vector<time_t>>>();
            auto back = messages->rbegin(); //Iteration goes backwards because messages are sorted descending
            //Order is very important here, because we get meaningless results if we're not moving the same direction time is
            for (auto it = back; it != messages->rend(); ++it){
                DLOG("strippedDates Processing: "+as_string(*it))
                if (it == back) continue; //Important so we don't try and do anything with the end iterator on the first pass
                if (back->sent == it->sent){
                    DLOG("Sent/Rec boolean equal to previous message")
                    //We know a strip needs to happen. The question is whether the new message is far enough ahead to strip the back one
                    if (it->date - back->date >= STRIP_RESET_DISTANCE){
                        DLOG("Temporal distance to previous message in excess of STRIP_RESET_DISTANCE, stripping. Set back to "+as_string(*it));
                        //This is too far, drop the old messages
                        back = it;
                    } //Else we keep going and have simply ignored the message with the same date
                } else {
                    //Sent and received differ, so we add a response time and move the back pointer up
                    DLOG(string("Sent/Rec boolean different from previous message, add ") + (it->sent ? "sent" : "received") +" response time"
                            " of "+as_string(it->date - back->date))
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

        void clearCache(){
            cacheLock.lock();
            cache.clear();
            cacheLock.unlock();
        }


        //For response times we compute both sent and rec, cache them separately, return the one requested

        shared_ptr<SentRec<vector<long>>> sentRecDenseTimes(const shared_ptr<const vector<Message>> query){
            auto sentRecDates = strippedDates(query);

            shared_ptr<SentRec<vector<long>>> ret = std::make_shared<SentRec<vector<long>>>();

            if (sentRecDates->received.size() > 0){
                ret->sent = denseTimes(sentRecDates->received);
            } else {
                logger::warning("Received dates for dense empty, returning empty vector");
            }

            if (sentRecDates->sent.size() > 0){
                ret->received = denseTimes(sentRecDates->sent);
            } else {
                logger::warning("Sent dates for dense time empty, returning empty vector");
            }

            return ret;
        }

        SentRec<double> conversationalResponseTimeCompute(const shared_ptr<const vector<Message>> query){
            auto sentRecDates = sentRecDenseTimes(query);

            double averageReceived = 0;
            double averageSent = 0;

            if (sentRecDates->sent.size() > 0){
                for (auto it = sentRecDates->sent.begin(); it != sentRecDates->sent.end(); ++it) averageReceived += *it;
                averageReceived /= (double)sentRecDates->sent.size();
            } else logger::warning("Received dates empty, returning 0");


            if (sentRecDates->received.size() > 0){
                for (auto it = sentRecDates->received.begin(); it != sentRecDates->received.end(); ++it) averageSent += *it;
                averageSent /= (double)sentRecDates->received.size();
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

        ConversationData fillInConvoData(shared_ptr<std::vector<Message>> query, long maxSentConvoResponse, long maxRecConvoResponse){
            ConversationData result;
            auto back = query->rbegin(); //Iteration goes backwards because messages are sorted descending
            //Order is very important here, because we get meaningless results if we're not moving the same direction time is
            auto convoStart = query->rend();

            vector<long> messageCounts, timeLengths, totalCharCounts;
            long messageCounter = 0, totalCharCounter = 0;
            bool conversation = false;

            /*
             * Keep a one-off starter pointer at the last message with the same sent/rec as current message.
             * Move it up if we hit a new message that is further than opposite (rec for sent, sent for rec) convoResponse f
             * rom the beginning, keep
             * moving it up until we get to a message within convoResponse from current. When we hit a message with
             * a sent/rec value different from current, we are either starting a conversation or logging all of our
             * one-offs.
             *
             * In convesation mode, we do the same thing until we hit a new message further than opposite convoResponse,
             * which ends the conversation. The last message in a conversation and similarly sent or rec messages before it
             * all count as the end of the convo, and not one-offs.
             *
             * Alternatively:
             * If we track the total number of sent messages, rec messages, sent messages in convos and rec messages
             * in convos, then we can just compute one-offs with subtraction. However, we're not doing this because
             * it's only marginally more efficient (same big O) and it removes the possobility of collecting data about
             * one-offs
             */
            for (auto it = back; it != query->rend(); ++it){
                if (it->date - back->date <= (it->sent ? maxSentConvoResponse : maxRecConvoResponse)){
                    //We're within conversational response range
                    if (it->sent == back->sent){
                        if (conversation){
                            //Consecutive conversation messages from the same person
                            messageCounter++;
                            totalCharCounter += it->charCount;
                        } else {
                            logger::info(""); //Need spoemthjing to dbug on
                            //We're looking at consecutive one-offs outside a conversation; we count these later
                        }
                    } else {
                        if (!conversation){
                            //We've detected a conversation, which we know to have started at "back"
                            //(because this is what we check against in the if)
                            messageCounter = 1;
                            totalCharCounter = it->charCount;
                            convoStart = back;
                            while(back != it){
                                //Messages before this are retroactively part of the conversation
                                messageCounter++;
                                totalCharCounter += back->charCount;
                                back++;
                            }
                            if (convoStart->sent) result.sentStartedCount++;
                            else result.recStartedCount++;
                            conversation = true;
                        } else {
                            //Normal conversational exchange
                            messageCounter++;
                            totalCharCounter += it->charCount;
                            back = it;
                        }
                    }
                } else {
                    //We're outside of conversational response range
                    if (conversation){
                        //Conversation ended at "back", and we're now outside of it
                        if (back->sent) result.recEndedCount++; //Person who fails to respond ends the conversation
                        else result.sentEndedCount++;
                        messageCounts.push_back(messageCounter);
                        totalCharCounts.push_back(totalCharCounter);
                        timeLengths.push_back(back->date - convoStart->date);
                        conversation = false;
                        back = it;
                    } else {
                        //one-offs before this, count them up
                        while (back != it){
                            if (back->sent) result.sentOneOffCount++;
                            else result.recOneOffCount++;
                            back++;
                        }
                    }
                }
            }

            if (conversation){
                //algorithm ended mid-conversation; no one has ended this yet, but we should still use metrics
                messageCounts.push_back(messageCounter);
                totalCharCounts.push_back(totalCharCounter);
                timeLengths.push_back(query->begin()->date - convoStart->date); //Normal begin will be last element in reverse
            } else {
                //one-offs before this, count them up
                while (back != query->rend()){
                    if (back->sent) result.sentOneOffCount++;
                    else result.recOneOffCount++;
                    back++;
                }
            }

            if (messageCounts.size() == 0){
                //No conversations found
                return result;
            }

            result.totalConversations = result.recStartedCount + result.sentStartedCount;

            result.averageMsgCount = std::accumulate(messageCounts.begin(), messageCounts.end(), 0)
                                     / (double)messageCounts.size();
            result.averageLengthTime = std::accumulate(timeLengths.begin(), timeLengths.end(), 0)
                                       / (double)timeLengths.size();
            result.averangeTotalChars = std::accumulate(totalCharCounts.begin(), totalCharCounts.end(), 0)
                                        / (double)totalCharCounts.size();
            return result;
        }


        ConversationData computeConvoData(AccountManager *account, const Contact *c, time_t fromDate, time_t untilDate){
            /*
             * We use the same dates to compute response time as we're given to search for conversations.
             * TODO: does that make sense?
             * We may just never call this on anything less than the full set of messages, though.
             */
            ConversationData result;
            auto query = reader::queryConversation(account, *c, KEY_DATE, fromDate, untilDate);
            auto conversationalResponseTimes = sentRecDenseTimes(query);
            if (conversationalResponseTimes->sent.size() == 0){
                logger::warning("Empty sent conversational response times in compute conversational data, returning 0s");
                return result;
            } else if (conversationalResponseTimes->received.size() == 0) {
                logger::warning("Empty received conversational response times in compute conversational data, returning 0s");
                return result;
            }

            long maxSentConvoResponse = *(std::max_element(conversationalResponseTimes->sent.begin(),
                                                           conversationalResponseTimes->sent.end()));
            long maxRecConvoResponse = *(std::max_element(conversationalResponseTimes->received.begin(),
                                                          conversationalResponseTimes->received.end()));

            if (maxSentConvoResponse == 0 || maxRecConvoResponse == 0){
                logger::warning("Insufficient convoersation response info to fill in conversation info");
            } else {
                result = fillInConvoData(query, maxSentConvoResponse, maxRecConvoResponse);
            }

            return result;
        }

        ConversationData conversationData(AccountManager *account, const Contact *c, time_t fromDate, time_t untilDate) {
            //Sent is hardcoded as true because we generate results for sent and received together
            if (c == NULL) throw queryException("Cannot run conversation queries with a null contact");
            if (countResult(CONVO_DATA, account, c, fromDate, untilDate, true)){
                return getResult<ConversationData>(CONVO_DATA, account, c, fromDate, untilDate, true);
            } else {
                ConversationData conversationData = computeConvoData(account, c, fromDate, untilDate);
                cacheResult<ConversationData>(CONVO_DATA, account, c, fromDate, untilDate, true, conversationData);
                return conversationData;
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