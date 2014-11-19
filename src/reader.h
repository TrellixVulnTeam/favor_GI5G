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
        DataLock<list<Contact>> contactList(const MessageType &t);

        shared_ptr<list<Address>> addresses(const MessageType &t); //We should really only need this internally
        bool addressExists(const string& addr, const MessageType &t);

        //Computation getters
        shared_ptr<vector<Message>> queryConversation(const AccountManager* account, const Contact& c, Key keys, time_t fromDate, time_t untilDate, bool sent);
        shared_ptr<vector<Message>> queryAll(const AccountManager* account, Key keys, time_t fromDate, time_t untilDate, bool sent);
        shared_ptr<vector<Message>> queryContact(const AccountManager* account, const Contact& c, Key keys, time_t fromDate, time_t untilDate, bool sent);
        long sum(const AccountManager* account, const Contact& c, Key key, time_t fromDate, time_t untilDate, bool sent);
        double average(const AccountManager* account, const Contact& c, Key key, time_t fromDate, time_t untilDate, bool sent);
        double averageAll(const AccountManager* account, const Key key, time_t fromDate, time_t untilDate, bool sent);
        long sumAll(const AccountManager* account, const Key key, time_t fromDate, time_t untilDate, bool sent);
        //TODO: a possible "queryAddresses" method and corresponding internal "multiQuery" ala the old favor, but I'm not sure we need it yet

        //Writers (the specific add/removes should be called exclusively by the worker)
        void removeAccount(AccountManager* account);
        void addAccount(AccountManager* account);
        void invalidateContactList(MessageType t);

        void refreshAll();

        void refreshAccountList();
        void refreshContactList(const MessageType &t);
    }
}

#endif