#include "accountmanager.h"
#include "worker.h"
#include "logger.h"

namespace favor{
  AccountManager::AccountManager(string accNm, MessageType typ, string detailsJson):type(typ),accountName(accNm){
    //TODO: parse json, with overriden virtual method if necessary
  }
  
  void AccountManager::buildTables()
  {
    worker::exec("CREATE TABLE IF NOT EXISTS "+SENT_TABLE_NAME SENT_TABLE_SCHEMA ";");
    worker::exec("CREATE TABLE IF NOT EXISTS "+RECEIVED_TABLE_NAME RECEIVED_TABLE_SCHEMA ";");
  }
    
  void AccountManager::truncateTables()
  {
    worker::exec("DELETE FROM "+SENT_TABLE_NAME);
    worker::exec("DELETE FROM "+RECEIVED_TABLE_NAME);
  }

    
  AccountManager AccountManager::buildManager(string accNm, MessageType typ, string detailsJson)
  {
    switch(typ){
      case TYPE_EMAIL:
	//stuff
	break;
      case TYPE_ANDROIDTEXT:
	break;
      default:
	logger::error("Attempt to initialize manager for unsupported type "+int_string(typ));
	//TODO: throw exception
    }

  }


}