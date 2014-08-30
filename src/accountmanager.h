#ifndef favor_accountmanager_include
#define favor_accountmanager_include

#include "favor.h"

namespace favor{
  
  
  class AccountManager{
  public:
    const MessageType type;
    const string accountName;
    

  protected:
    AccountManager(string accNm, MessageType typ, string detailsJson);
    void holdMessage(bool sent, long id, long date, string address, bool media, string msg);
    void exportHeldMessages();
  public:
    void buildTables();
    void truncateTables();
    void indexTables();
    void deindexTables();
  //Static methods
    static AccountManager buildManager(string accNm, MessageType typ, string detailsJson);
  };
  
}

#endif