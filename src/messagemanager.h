#ifndef favor_messagemanager_include
#define favor_messagemanager_include

#include "favor.h"


namespace favor{
  class MessageManager{
  private:
    MessageType type;
    string accountName;
  protected:
    MessageManager(MessageType typ, string accNm);
    void holdMessage(bool sent, long id, long date, string address, bool media, string msg);
    void exportHeldMessages();
  public:
    void buildTables();
    void truncateTables();
    void indexTables();
    void deindexTables();
  };
  
}

#endif