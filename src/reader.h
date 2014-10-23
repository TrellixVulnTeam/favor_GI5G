#ifndef favor_reader_include
#define favor_reader_include

#include "favor.h"
#include "accountmanager.h"

namespace favor {
    namespace reader {
        //Basic
        void initialize();

        void cleanup();

        //Getters
        list<AccountManager*> accountList();
        list<Contact*> contactList(const MessageType &t);

        shared_ptr<list<Address>> addresses(const MessageType &t); //We should really only need this internally

        //Writers
        void removeAccount(AccountManager* account);
        void addAccount(AccountManager* account);

        void refreshAll();

        void refreshAccountList();
        void refreshContactList(const MessageType &t);
    }
}

#endif