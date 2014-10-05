#include "accountmanager.h"
#include "worker.h"
#include "logger.h"

//Managers
#ifdef FAVOR_EMAIL_MANAGER
#include "emailmanager.h"
#endif

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
  
  void AccountManager::truncateSentTable()
  {
    worker::exec("DELETE FROM " SENT_TABLE_NAME);
  }
  
  void AccountManager::truncateReceivedTable()
  {
    worker::exec("DELETE FROM " RECEIVED_TABLE_NAME);
  }
  
  void AccountManager::truncateTables()
  {
    truncateSentTable();
    truncateReceivedTable();
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
  
  void AccountManager::saveFetchData()
  {
    sqlite3_stmt *stmt;
    //TODO: more hardcoded column names :(
    //TODO: this may have to be delegated to worker because it would benefit from having a DB pointer
//     const char sql[] = "UPDATE " ACCOUNT_TABLE " SET details_json=? WHERE name=? AND type=?;";
//     sqlv(sqlite3_prepare_v2(db, sql, sizeof(sql), &stmt, NULL));
//     //TODO: json to string
//     //sqlv(sqlite3_bind_text(stmt, 1, json.
//     sqlv(sqlite3_bind_text(stmt, 2, accountName.c_str(), accountName.length(), SQLITE_STATIC));
//     sqlv(sqlite3_bind_int(stmt, 3, type));
//     sqlv(sqlite3_step(stmt));
//     sqlv(sqlite3_finalize(stmt));
  }

  
  void AccountManager::saveHeldMessages()
  {
    //TODO: save the messages we have to the database
  }

  void AccountManager::updateMessages()
  {
    fetchMessages();
    updateFetchData();
    saveHeldMessages();
    saveFetchData();
  }
  
  void AccountManager::holdMessage(const bool sent, const long int id, const std::time_t date, const string address, const bool media, const string& msg, favor::Encoding enc)
  {
    //TODO: export (save to vector, likely of pointers), and give it a type based on the type of this manager. also eventually worry about encodings
    //TODO: in one pass, strip any whitespace which is either consecutive or trailing, compute unicode string length, and copy the string
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