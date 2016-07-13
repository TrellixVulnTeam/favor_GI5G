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



#ifndef favor_reader_include
#define favor_reader_include

#include "favor.h"
#include "accountmanager.h"
#include "datalock.h"
#include "logger.h"

namespace favor {
    namespace reader {
        //Basic
        void initialize();

        void cleanup();

        //Getters
        DataLock<list<AccountManager*>> accountList();
        DataLock<list<Contact>> contactList();

        shared_ptr<list<Address>> addresses(const MessageTypeFlag &ts, bool contactRelevantOnly = false); //We should really only need these internally
        shared_ptr<list<Address>> addresses(const MessageType &t, bool contactRelevantOnly = false);
        bool addressExists(const string& addr, const MessageType &t);

        //Computation getters
        shared_ptr<vector<Message>> queryConversation(const AccountManager* account, const Contact& c, Key keys, time_t fromDate, time_t untilDate);
        shared_ptr<vector<Message>> queryAll(const AccountManager* account, Key keys, time_t fromDate, time_t untilDate, bool sent);
        shared_ptr<vector<Message>> queryContact(const AccountManager* account, const Contact& c, Key keys, time_t fromDate, time_t untilDate, bool sent);
        double average(const AccountManager* account, const Contact& c, Key key, time_t fromDate, time_t untilDate, bool sent);
        double averageAll(const AccountManager* account, const Key key, time_t fromDate, time_t untilDate, bool sent);
        long sum(const AccountManager* account, const Contact& c, Key key, time_t fromDate, time_t untilDate, bool sent);
        long sumAll(const AccountManager* account, const Key key, time_t fromDate, time_t untilDate, bool sent);
        long count(const AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);
        long countAll(const AccountManager* account, time_t fromDate, time_t untilDate, bool sent);
        long max(const AccountManager* account, const Contact& c, Key key, time_t fromDate, time_t untilDate, bool sent);
        //TODO: a possible "queryAddresses" method and corresponding internal "multiQuery" ala the old favor, but I'm not sure we need it yet

        //Writers (the specific add/removes should be called exclusively by the worker)
        void removeAccount(AccountManager* account);
        void addAccount(AccountManager* account);
        void invalidateContactList();

        void refreshAll();

        void refreshAccountList();
        void refreshContactList();
    }
}

#endif