#ifndef favor_accountmanager_include
#define favor_accountmanager_include

#include "favor.h"

namespace favor{
  
  
  class AccountManager{
  public:
    const MessageType type;
    const string accountName;
    
  private:
    void exportHeldMessages();
    
  protected:
    AccountManager(string accNm, MessageType typ, string detailsJson);
    void holdMessage(bool sent, long id, long date, string address, bool media, string msg);
    virtual void fetchMessages();
    virtual void fetchContacts();
  public:
    //Database
    void buildTables();
    void destroyTables();
    void truncateTables();
    void indexTables();
    void deindexTables();
    //Work
    void updateMessages();
    void updateContacts();
  //Static methods
    static AccountManager buildManager(string accNm, MessageType typ, string detailsJson);
    static void buildTablesStatic(string accountName, MessageType type);
    static void destroyTablesStatic(string accountName, MessageType type);
  };
  
}

#endif