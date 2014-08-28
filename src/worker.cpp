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
    
    void sqlite3_exec_noread(string sql){
      char *err = NULL;
      int result = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err);
      if (result!=SQLITE_OK){
	//log sqlite3_errstr(result)
	sqlite3_free(err);
	//TODO: throw new exception with the error string in err instead of using cout, just make sure we keep the free call so we don't leak
      }
    }
    
    void add_account(string name, MessageType type, string detailsJson)
    {
      sqlite3_stmt *stmt;
      //TODO: Investigate how necessary freeing things is when binding text to sqlite, and make sure that we know what to avoid
      //in this method and methods like it when it comes to leaking memory
      sqlv(sqlite3_prepare_v2(db, "INSERT INTO " ACCOUNT_TABLE " VALUES(?,?,?);", -1, &stmt, NULL));
      sqlv(sqlite3_bind_text(stmt, 1, name.c_str(), strlen(name.c_str()), SQLITE_STATIC)); //Memory managed by containing string, so we tell SQLite it's static
      sqlv(sqlite3_bind_int(stmt, 2, type));
      sqlv(sqlite3_bind_text(stmt, 3, detailsJson.c_str(), strlen(detailsJson.c_str()), SQLITE_STATIC));
      sqlv(sqlite3_step(stmt));
      sqlv(sqlite3_finalize(stmt));
    }

    
    void buildDatabase(){
      sqlite3_exec_noread("CREATE TABLE IF NOT EXISTS " ACCOUNT_TABLE ACCOUNT_TABLE_SCHEMA ";");
      //TODO: build contacts/addresses by type, and messages/data by account. see design doc.
    }
    
  }
  
}