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
    void saveHeldMessages();
    
  protected:
    AccountManager(string accNm, MessageType typ, string detailsJson);
    AccountManager(const AccountManager& that) = delete; //This shouldn't ever be copied. 
    void truncateSentTable();
    void truncateReceivedTable();
    void holdMessage(const bool sent, const long int id, const std::time_t date, const favor::string address, const bool media, const favor::string& msg, favor::Encoding enc = ASCII);
    virtual void fetchMessages() = 0;
    virtual void fetchContacts() = 0;
    virtual void updateFetchData() = 0;
    void saveFetchData();
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