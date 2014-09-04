#include "accountmanager.h"
#include "./managers/managers.h"
#include "worker.h"
#include "logger.h"

#define SENT_TABLE_NAME "\""+accountName+"_"+MessageTypeName[type]+"_sent\""
#define RECEIVED_TABLE_NAME "\""+accountName+"_"+MessageTypeName[type]+"_received\""

#define SENT_INDEX_NAME "i_" SENT_TABLE_NAME
#define RECEIVED_INDEX_NAME "i_" RECEIVED_TABLE_NAME

namespace favor{
  AccountManager::AccountManager(string accNm, MessageType typ, string detailsJson):type(typ),accountName(accNm){
    //TODO: parse json, with overriden virtual method if necessary

  }
  
  void AccountManager::buildTables()
  {
    buildTablesStatic(accountName, type);
  }
  
  void AccountManager::destroyTables()
  {
    destroyTablesStatic(accountName, type);
  }

    
  void AccountManager::truncateTables()
  {
    worker::exec("DELETE FROM " SENT_TABLE_NAME);
    worker::exec("DELETE FROM " RECEIVED_TABLE_NAME);
  }
  
  void AccountManager::deindexTables()
  {
    worker::exec("DROP INDEX IF EXISTS " SENT_INDEX_NAME ";");
    worker::exec("DROP INDEX IF EXISTS " RECEIVED_INDEX_NAME ";");
  }
  
  void AccountManager::indexTables()
  {
    worker::exec("CREATE INDEX IF NOT EXISTS " RECEIVED_INDEX_NAME " ON " RECEIVED_TABLE_NAME MESSAGE_INDEX_SCHEMA ";");
    worker::exec("CREATE INDEX IF NOT EXISTS " SENT_INDEX_NAME " ON " SENT_TABLE_NAME MESSAGE_INDEX_SCHEMA ";");
  }
  
  void AccountManager::fetchContacts()
  {
    logger::error("Unoverridden fetchContacts() called. This should never happen.");
    assert(false);
  }

  void AccountManager::fetchMessages()
  {
    logger::error("Unoverridden fetchMessages() called. This should never happen.");
    assert(false);
  }

  //Static methods

    
  AccountManager AccountManager::buildManager(string accNm, MessageType typ, string detailsJson)
  {
    switch(typ){
      case TYPE_EMAIL: return EmailManager(accNm, detailsJson);
      case TYPE_ANDROIDTEXT:
	break;
      default:
	logger::error("Attempt to initialize manager for unsupported type "+as_string(typ));
	assert(false);
    }

  }
  
  void AccountManager::buildTablesStatic(string accountName, MessageType type)
  {
    //TODO: index if indexing is enabled
    worker::exec("CREATE TABLE IF NOT EXISTS " SENT_TABLE_NAME SENT_TABLE_SCHEMA ";");
    worker::exec("CREATE TABLE IF NOT EXISTS " RECEIVED_TABLE_NAME RECEIVED_TABLE_SCHEMA ";");
  }

  void AccountManager::destroyTablesStatic(string accountName, MessageType type)
  {
    worker::exec("DROP TABLE IF EXISTS " SENT_TABLE_NAME ";");
    worker::exec("DROP TABLE IF EXISTS " RECEIVED_TABLE_NAME ";");
  }


}