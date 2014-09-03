#include "favor.h"
#include "worker.h"
#include "reader.h"
#include "logger.h"

using namespace std;

namespace favor{
  const char* MessageTypeName[] = {"email", "androidtext", "line", "skype"};
  
  void initialize(){
    sqlite3_enable_shared_cache(1);
    //The worker must be initialized first, because it will create the database if there is none
    worker::initialize();
    reader::initialize();
  }
  
  void cleanup(){
   worker::cleanup();
   reader::cleanup();
  }
  

  void sqlite3_validate(int result, sqlite3 *db ){
    switch(result){
      //TODO: a specific case for constraint violations (esp. unique) that logs an error but doesn't except
      case SQLITE_OK: break;
      case SQLITE_ROW: break;
      case SQLITE_DONE: break;
      default:
	logger::error("SQLite error #"+to_string(result)+", db says \""+sqlite3_errmsg(db)+"\"");
	throw sqliteException();
    }
  }

}