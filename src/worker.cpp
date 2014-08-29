#include "worker.h"

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
      sqlv(sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL)); //TODO: make sure this errors properly (might behave differently since it's intended to use the 5th param for err out?)
    }
    
    void addAccount(string name, MessageType type, string detailsJson)
    {
      sqlite3_stmt *stmt;
      //TODO: Investigate how necessary freeing things is when binding text to sqlite, and make sure that we know what to avoid
      //in this method and methods like it when it comes to leaking memory. also: strlen() vs string.length() ??
      
      //TODO: does this work binding after passing sizeof? I assume so, but...
      const char* sql = "INSERT INTO " ACCOUNT_TABLE " VALUES(?,?,?);";
      sqlv(sqlite3_prepare_v2(db, sql, sizeof(sql), &stmt, NULL));
      sqlv(sqlite3_bind_text(stmt, 1, name.c_str(), strlen(name.c_str()), SQLITE_STATIC)); //Memory managed by containing string, so we tell SQLite it's static
      sqlv(sqlite3_bind_int(stmt, 2, type));
      sqlv(sqlite3_bind_text(stmt, 3, detailsJson.c_str(), strlen(detailsJson.c_str()), SQLITE_STATIC));
      sqlv(sqlite3_step(stmt));
      sqlv(sqlite3_finalize(stmt));
    }

    
    void buildDatabase(){
      exec("CREATE TABLE IF NOT EXISTS " ACCOUNT_TABLE ACCOUNT_TABLE_SCHEMA ";");
      //TODO: build contacts/addresses by type, and messages/data by account. see design doc.
    }
    
    void truncateDatabase()
    {
      /* http://www.sqlite.org/lang_delete.html
      * When the WHERE is omitted from a DELETE statement and the table being deleted has no triggers, 
      * SQLite uses an optimization to erase the entire table content without having to visit each row of the table individually. 
      */
      exec("DELETE FROM " ACCOUNT_TABLE);
      //TODO: truncate contacts/addresses by type, and messages/data by account.
    }

    
  }
  
}