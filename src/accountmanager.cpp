#include "accountmanager.h"
#include "./managers/managers.h"
#include "worker.h"
#include "logger.h"

#define SENT_TABLE_NAME "\""+accountName+"_"+MessageTypeName[type]+"_sent\""
#define RECEIVED_TABLE_NAME "\""+accountName+"_"+MessageTypeName[type]+"_received\""

#define SENT_INDEX_NAME "i_" SENT_TABLE_NAME
#define RECEIVED_INDEX_NAME "i_" RECEIVED_TABLE_NAME

using namespace std;
namespace favor{
  AccountManager::AccountManager(string accNm, MessageType typ, string detailsJson):type(typ),accountName(accNm){
    json.Parse(detailsJson.c_str());
    if (json.HasParseError()){
      logger::error("Parse error on json: \""+detailsJson+"\". RapidJson says: "+rapidjson::GetParseError_En(json.GetParseError()));
      throw badAccountDataException("Failed to parse JSON details");
    }
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
  
  void AccountManager::updateContacts()
  {
    fetchContacts();
    //TODO: process results
  }

  void AccountManager::updateMessages()
  {
    fetchMessages();
    //TODO: process results
  }
  
  void AccountManager::holdMessage(const bool sent, const long int id, const std::time_t date, const string address, const bool media, const string msg, favor::Encoding enc)
  {
    //TODO: actually export message, and give it a type based on the type of this manager
      cout << "---------------------------------------------------------" << endl;
      cout << "Message held with - sent: " << sent << ", id: " << id << ", date: " << date << ", address: " << address << ", media: " << media << endl << "...and Body:|" << msg << "|" << endl;
      cout << "Body　Length: " << msg.length() << ", Encoding: " << enc << endl;
      cout << "---------------------------------------------------------" << endl << endl;
      
  }

  //Static methods

    
  shared_ptr<AccountManager> AccountManager::buildManager(string accNm, favor::MessageType typ, string detailsJson)
  {
    switch(typ){
      #ifdef FAVOR_EMAIL_MANAGER
      case TYPE_EMAIL: return make_shared<EmailManager>(accNm, detailsJson);
      #endif
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