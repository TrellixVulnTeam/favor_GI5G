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

        //Writers
        void removeAccount(AccountManager* account);

        void refreshAll();

        void refreshAccountList();
    }
}

#endif