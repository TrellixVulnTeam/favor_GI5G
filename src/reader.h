#ifndef favor_reader_include
#define favor_reader_include

#include "favor.h"
#include "accountmanager.h"

namespace favor{
  namespace reader{
    //Basic
    void initialize();
    void cleanup();
    //Getters
    list<shared_ptr<AccountManager>> accountList();
    //Refreshers
    void refreshAll();
    void refreshAccountList();
  }
}

#endif