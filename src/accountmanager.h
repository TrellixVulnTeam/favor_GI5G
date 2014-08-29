#ifndef favor_accountmanager_include
#define favor_accountmanager_include

#include "favor.h"

#define SENT_TABLE_NAME accountName+"_"+MessageTypeName[type]+"_sent"
#define RECEIVED_TABLE_NAME accountName+"_"+MessageTypeName[type]+"_received"

namespace favor{
  class AccountManager{
  private:
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