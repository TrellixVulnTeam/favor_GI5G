#ifndef favor_accountmanager_include
#define favor_accountmanager_include

#include "favor.h"

namespace favor{
  
  
  class AccountManager{
  public:
    const MessageType type;
    const string accountName;
    
  protected:
    rapidjson::Document json;
    
  private:
    void exportHeldMessages();
    
  protected:
    AccountManager(string accNm, MessageType typ, string detailsJson);
    AccountManager(const AccountManager& that) = delete; //This shouldn't ever be copied. 
    void holdMessage(const bool sent, const long id, const std::time_t date, const string address, const bool media, const string msg, Encoding enc = ASCII);
    virtual void fetchMessages() = 0;
    virtual void fetchContacts() = 0;
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
    static shared_ptr<AccountManager> buildManager(string accNm, MessageType typ, string detailsJson);
    static void buildTablesStatic(string accountName, MessageType type);
    static void destroyTablesStatic(string accountName, MessageType type);
  };
  
}

#endif