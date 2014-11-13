#ifndef favor_reader_include
#define favor_reader_include

#include "favor.h"
#include "accountmanager.h"
#include "datalock.h"

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

        //Writers (the specific add/removes should be called exclusively by the worker)
        void removeAccount(AccountManager* account);
        void addAccount(AccountManager* account);
        void addContact(Contact& contact);
        void removeContact(Contact& contact);

        void refreshAll();

        void refreshAccountList();
        void refreshContactList(const MessageType &t);
    }
}

#endif