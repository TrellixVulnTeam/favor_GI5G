#include "worker.h"
#include "reader.h"

namespace favor{
  namespace worker{
    namespace{  //Private members
      sqlite3 *db;
    }
    void initialize(){
      sqlv(sqlite3_open(DB_NAME, &db));
    }
    
    void cleanup(){
      sqlv(sqlite3_close(db));
    }
    
    void exec(string sql){
      sqlv(sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL));
    }
    
    void addAccount(string name, MessageType type, string detailsJson)
    {
      sqlite3_stmt *stmt;
      const char sql[] = "INSERT INTO " ACCOUNT_TABLE " VALUES(?,?,?);"; //Important this is an array and not a const char* so that sizeof() works properly
      sqlv(sqlite3_prepare_v2(db, sql, sizeof(sql), &stmt, NULL));
      sqlv(sqlite3_bind_text(stmt, 1, name.c_str(), name.length(), SQLITE_STATIC)); //Memory managed by containing string, so we tell SQLite it's static
      sqlv(sqlite3_bind_int(stmt, 2, type));
      sqlv(sqlite3_bind_text(stmt, 3, detailsJson.c_str(), detailsJson.length(), SQLITE_STATIC));
      sqlv(sqlite3_step(stmt));
      sqlv(sqlite3_finalize(stmt));
    }

    
    void buildDatabase(){
      exec("CREATE TABLE IF NOT EXISTS " ACCOUNT_TABLE ACCOUNT_TABLE_SCHEMA ";");
      for(int i = 0; i < NUMBER_OF_TYPES; ++i){
	//TODO: build contacts/addresses by type
      }
      list<AccountManager> l = reader::accountList();
      for(list<AccountManager>::iterator it = l.begin(); it != l.end(); ++it){
	it->buildTables();
      }
    }
    
    void truncateDatabase()
    {
      /* http://www.sqlite.org/lang_delete.html
      * When the WHERE is omitted from a DELETE statement and the table being deleted has no triggers, 
      * SQLite uses an optimization to erase the entire table content without having to visit each row of the table individually. 
      */
      exec("DELETE FROM " ACCOUNT_TABLE);
      for(int i = 0; i < NUMBER_OF_TYPES; ++i){
	//TODO: truncate contacts/addresses by type
      }
      list<AccountManager> l = reader::accountList();
      for(list<AccountManager>::iterator it = l.begin(); it != l.end(); ++it){
	it->truncateTables();
      }
    }
    
    void indexDatabase()
    {

    }
    
    void deindexDatabase()
    {

    }



    
  }
  
}