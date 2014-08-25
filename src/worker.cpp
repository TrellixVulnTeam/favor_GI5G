#include "worker.h"
#include <iostream> //TODO: REMOVE THIS

namespace favor{
  namespace worker{
    namespace{  //Private members
      sqlite3 *db;
      void sqlite3_exec_noread(const char *sql){
	char *err = NULL;
	int res = sqlite3_exec(db, sql, NULL, NULL, &err);
	if (res!=SQLITE_OK){
	  std::cout << sql << std::endl;
	  std::cout << err << std::endl;
	  //TODO: throw new exception with the error string in err instead of using cout
	}
      }
    }
    void initialize(){
      int result = sqlite3_open(DB_NAME, &db);
      if (result!=SQLITE_OK){
	//TODO: throw an exception or something
      }
      
    }
    
    void cleanup(){
      int result = sqlite3_close(db);
      if (result!=SQLITE_OK){
	//Log failure, probably means it's busy because we haven't finalized all of our statements
      }
    }
    
    void buildDatabase(){
      sqlite3_exec_noread("CREATE TABLE IF NOT EXISTS " ACCOUNT_TABLE ACCOUNT_TABLE_SCHEMA ";");
      //Todo: build contacts/addresses by type, and messages/data by account. see design doc.
    }
    
  }
  
}