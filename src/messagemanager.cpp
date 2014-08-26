#include "messagemanager.h"
#include "worker.h"

namespace favor{
    MessageManager::MessageManager(MessageType typ, string accNm){
      type = typ;
      accountName = accNm;
    }
    
    void MessageManager::buildTables()
    {
      //TODO: this function
      //worker::sqlite3_exec_noread();
    }

}